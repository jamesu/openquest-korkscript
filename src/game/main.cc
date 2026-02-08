//-----------------------------------------------------------------------------
// Copyright (c) 2025-2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// 
#include "platform/platform.h"
#include "raylib.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "math/mathTypes.h"
#include "sim/simBase.h"
#include "core/freeListHandleHelpers.h"
#include "core/memStream.h"
#include "core/fileStream.h"
#include <math.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// --- Tweak these ---
#define TICK_HZ        128.0        // fixed simulation rate (e.g. 60, 64, 128)
#define MAX_FRAME_DT   0.25         // clamp to avoid spiral-of-death (seconds)
#define MAX_STEPS      8            // safety cap: max sim steps per render frame

typedef struct State {
    Vector2 pos;
    Vector2 vel;
    float rotationDeg;
} State;

typedef struct Input {
    int moveX;   // -1, 0, +1
    int moveY;   // -1, 0, +1
} Input;

// TextureManager.h/.cpp (single-file style)
#include "raylib.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <algorithm>

#include "math/mPoint.h"
#include "math/mRect.h"

class TextureHandle;

struct ColorI
{
   U8 r;
   U8 g;
   U8 b;
   U8 a;
};

class TextureSlot
{
    friend class TextureHandle;
    friend class TextureManager;
    friend class FreeListPtr<TextureSlot, TextureHandle, std::vector>;
    friend class FreeListHandle::Basic32;

public:
   U32 mAllocNumber;
   U8 mGeneration : 7;
   S32 mRefCount;

   ::Texture2D mTexture;
   std::string mPath;

   TextureSlot() : mAllocNumber(0), mGeneration(0), mTexture({}), mRefCount(0) {;}


    void incRef()
    {
        mRefCount++;
    }

    void decRef()
    {
        if (mRefCount > 0) mRefCount--;
    }

    void reset()
    {
        if (mTexture.id != 0)
        {
            ::UnloadTexture(mTexture);
        }
        mPath = "";
    }

};

class TextureHandle : public FreeListHandle::Basic32
{
public:
   TextureHandle() { parts.heavyRef = (U32)1; }
   TextureHandle(std::nullptr_t) { parts.heavyRef = (U32)1; }
   explicit TextureHandle(U32 fullAllocNumber, bool isHeavy=true)
   {
      value = fullAllocNumber;
      parts.heavyRef = (U32)isHeavy;
      if (isHeavy)
      {
         TextureSlot* obj = getPtr();
         if (obj)
            obj->incRef();
      }
   }
   explicit TextureHandle(U32 num, U8 gen, bool isHeavy=true) : Basic32(num, gen, isHeavy)
   {
      if (isHeavy)
      {
         TextureSlot* obj = getPtr();
         if (obj)
            obj->incRef();
      }
   }
   TextureHandle(const TextureHandle& other)
   {
      TextureSlot* newTO = other.getPtr();
      if (other.isHeavyRef())
      {
         newTO->incRef();
      }
      value = other.value;
   }
   TextureHandle(TextureSlot* slot, bool isHeavy=true) : FreeListHandle::Basic32(0, isHeavy)
   {
    setSlot(slot);
   }
   ~TextureHandle()
   {
      if (parts.index != 0)
      {
         setSlot(nullptr);
      }
   }

   void setSlot(TextureSlot* newSlot)
   {
      if (isHeavyRef())
      {
         TextureSlot* existSlot = getPtr();
         if (existSlot != newSlot)
         {
            if (newSlot != nullptr)
               newSlot->incRef();
            if (existSlot != nullptr)
               existSlot->decRef();
         }
      }
      value = newSlot != nullptr ? TextureHandle::makeValue(newSlot->mAllocNumber, newSlot->mGeneration, isHeavyRef()) :
                                TextureHandle::makeValue(0,0, isHeavyRef());
   }
   
   inline TextureHandle& operator=(std::nullptr_t) {
      setSlot(nullptr);
      return *this;
   }
   
   inline TextureHandle& operator=(const TextureHandle& other) {
      setSlot(other.getPtr());
      return *this;
   }
   
   inline bool operator==(const TextureHandle& other) const {
      return value == other.value;
   }
   inline bool operator!=(const TextureHandle& other) const {
      return value != other.value;
   }
   

   TextureSlot* getPtr() const;
};

class TextureManager 
{
public:
    FreeListPtr<TextureSlot, TextureHandle, std::vector> mTextureList;

    TextureManager() = default;
    ~TextureManager() { cleanup(); }

    TextureHandle loadTexture(const std::string& path) 
    {
        // Already loaded?
        auto it = mPathToId.find(path);
        if (it != mPathToId.end()) {
            U32 id = it->second;
            return TextureHandle(id);
        }

        // Load new texture
        ::Texture2D tex = ::LoadTexture(path.c_str());

        TextureSlot* slot = new TextureSlot();
        slot->mTexture = tex;
        slot->mPath = path;

        mTextureList.allocListHandle(slot, false);

        U32 newId = TextureHandle::makeValue(slot->mAllocNumber, slot->mGeneration, true);
        mPathToId[path] = newId;
        return TextureHandle(slot);
    }

    inline TextureSlot* resolveHandle(const TextureHandle& h)
    {
        return mTextureList.getItem(h.getValue());
    }

    void flushUnused()
    {
       mTextureList.forEach([this](TextureSlot* slot){
          if (slot->mRefCount == 0)
          {
            mTextureList.freeListPtr(slot);
          }
       });
    }

    void cleanup() 
    {
       mTextureList.forEach([this](TextureSlot* slot){
        mTextureList.freeListPtr(slot);
       });
    }

private:

    std::unordered_map<std::string, U32> mPathToId; // path -> slot handle
    std::vector<TextureSlot*> mSlots;                             // id-1 indexing
};

TextureManager* gTextureManager = nullptr;

TextureSlot* TextureHandle::getPtr() const
{
    return gTextureManager->resolveHandle(*this);
}

struct RoomRender
{
   enum TransitionMode : U8
   {
      TRANSITION_NONE,
      TRANSITION_IRIS_IN,  // scissor towards center + shadow border, blank [then flip gfx after]
      TRANSITION_IRIS_OUT, // inverse scissor towards center + shadow border [no gfx change]
      TRANSITION_WIPE_HORIZONTAL_IN, // scissor horizontal
      TRANSITION_WIPE_HORIZONTAL_OUT, // scissor horizontal
      TRANSITION_WIPE_VERTICAL_IN, // scissor vertical
      TRANSITION_WIPE_VERTICAL_OUT // scissor vertical
   };
   
   enum TransitionWipeOrigin
   {
      WIPE_ORIG_LEFT,
      WIPE_ORIG_RIGHT,
      // aliases for above
      WIPE_ORIG_TOP = WIPE_ORIG_LEFT,
      WIPE_ORIG_BOTTOM = WIPE_ORIG_RIGHT
   };
   
   struct TransitionParams
   {
      U8 mode : 4;
      U8 params : 4;
   };
   
   enum
   {
       NumZPlanes = 3
   };
    TextureHandle backgroundImage;
    TextureHandle zPlanes[3];
   
   Point2I screenSize;
   RectI backgroundSR;
   RectI clipRect;
   ColorI bgColor;
   
   TransitionParams currentTransition;
   F32 transitionPos;
   F32 transitionTime;
   
   static float smoothstep(float t) {
       // clamp
       if (t < 0) t = 0;
       if (t > 1) t = 1;
       return t*t*(3.0f - 2.0f*t);
   }

   static RectI computeWipeRect(int W, int H, float t01, TransitionMode mode, TransitionWipeOrigin origin)
   {
       float p = smoothstep(t01);
       float s = (mode == TRANSITION_WIPE_HORIZONTAL_IN ||
                  mode == TRANSITION_WIPE_VERTICAL_IN) ? p : (1.0f - p);  // size fraction: grows for IN, shrinks for OUT

       float x = 0, y = 0, w = (float)W, h = (float)H;

       if (mode == TRANSITION_WIPE_HORIZONTAL_IN || mode == TRANSITION_WIPE_HORIZONTAL_OUT) {
           w = s * W;

           if (origin == WIPE_ORIG_LEFT) {
               x = 0;
           } else { // WIPE_ORIG_RIGHT
               x = W - w;
           }
           y = 0; h = H;
       } else { // AXIS_V
           h = s * H;

           if (origin == WIPE_ORIG_TOP) {
               y = 0;
           } else { // WIPE_ORIG_BOTTOM
               y = H - h;
           }
           x = 0; w = W;
       }

       // Avoid negative/NaN and tiny seams: clamp + round outward a bit
       if (w < 0) w = 0; if (h < 0) h = 0;
       if (x < 0) x = 0; if (y < 0) y = 0;
       if (x + w > W) w = W - x;
       if (y + h > H) h = H - y;

       return RectI(x,y,w,h);
   }
   
   void updateTransition(float dt)
   {
      transitionPos += dt * (1.0/transitionTime);
      
      if (transitionPos > 1.0)
      {
         transitionPos = 1.0;
      }
      
      Point2F halfSize = Point2F(screenSize.x, screenSize.y) * 0.5f;
      Point2I fullSize = Point2I(screenSize.x, screenSize.y);
      
      switch (currentTransition.mode)
      {
         case TRANSITION_IRIS_IN:
            clipRect.point = (halfSize * transitionPos).asPoint2I();
            clipRect.extent = (fullSize - clipRect.point) - (halfSize * transitionPos).asPoint2I();
            break;
         case TRANSITION_IRIS_OUT:
            clipRect.point = (halfSize * (1.0f-transitionPos)).asPoint2I();
            clipRect.extent = (fullSize - clipRect.point) - (halfSize * (1.0f-transitionPos)).asPoint2I();
            break;
         case TRANSITION_WIPE_HORIZONTAL_IN:
         case TRANSITION_WIPE_HORIZONTAL_OUT:
         case TRANSITION_WIPE_VERTICAL_IN:
         case TRANSITION_WIPE_VERTICAL_OUT:
            clipRect = computeWipeRect(screenSize.x,
                                       screenSize.y,
                                       transitionPos,
                                       (TransitionMode)currentTransition.mode,
                                       (TransitionWipeOrigin)currentTransition.params);
            break;
         default:
            clipRect = RectI(0,0,screenSize.x, screenSize.y);
            break;
            
      }
   }

};

struct CostumeRenderer
{
    enum Flags
    {
        LOOP=BIT(0),
        PING_PONG=BIT(1)
    };

    enum
    {
        NumDirections = 4
    };

    // Limb track for direction
    struct AnimDirection
    {
        // target limb
        U32 startLimbMap; // index into AnimLimbMap
        U32 numLimbs;     // x NumDirections
        U32 flags;        // loop, etc
    };

    struct AnimInfo
    {
        AnimDirection directionTracks[NumDirections]; // tracks for each dir
        U32 flags; // loop, etc
    };

    struct AnimLimbMap
    {
        U32 targetLimb; // limb the frames are for
        U32 startFrame; // index into Frame list
        U32 numFrames;  // frames to use
    };

    // compiled frame
    struct Frame
    {
        TextureHandle displayImage;
        Vector2 offset;
        U32 flags;
    };

    // State of limbs
    struct LimbState
    {
        StringTableEntry name;
        U32 startFrame;    // start frame of current loop
        U32 numFrames;     // 0 = invisible
        U32 lastEvalFrame; // last frame we are displaying
    };

    // compiled state
    struct StaticState
    {
        std::vector<Frame> mFrames;
        std::vector<AnimLimbMap> mLimbMap;
        std::vector<AnimInfo> mAnims;
        std::vector<LimbState> mBaseState;
       
       void reset()
       {
          mFrames.clear();
          mLimbMap.clear();
          mAnims.clear();
          mBaseState.clear();
       }
    };

    // live state
    struct LiveState
    {
        U8 flags;
        U8 curAnim;      // AnimInfo we are playing
        U8 curTalkAnim;  // AnimInfo for talk (override)
        U8 curAnimFrame; // tick for anim
        U8 curDirection; // current direction
        Vector2 position; // display position
        std::vector<LimbState> mLimbState;
       
       void reset()
       {
       }
    };
};


static Input SampleInput(void)
{
    Input in = { 0 };

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  in.moveX -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) in.moveX += 1;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    in.moveY -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  in.moveY += 1;

    return in;
}

// Fixed-step simulation update. dt is constant: 1/TICK_HZ
static void UpdateFixed(State *s, Input in, float dt)
{
    const float moveSpeed = 300.0f;  // pixels/sec
    const float rotateSpeed = 90.0f; // deg/sec

    // Build desired velocity from input (normalize diagonals)
    Vector2 v = { (float)in.moveX, (float)in.moveY };
    float len = sqrtf(v.x*v.x + v.y*v.y);
    if (len > 0.0f) {
        v.x /= len;
        v.y /= len;
    }

    s->vel.x = v.x * moveSpeed;
    s->vel.y = v.y * moveSpeed;

    s->pos.x += s->vel.x * dt;
    s->pos.y += s->vel.y * dt;

    s->rotationDeg += rotateSpeed * dt;
}

static State LerpState(State a, State b, float alpha)
{
    State out = {0};
    out.pos.x = a.pos.x + (b.pos.x - a.pos.x) * alpha;
    out.pos.y = a.pos.y + (b.pos.y - a.pos.y) * alpha;
    out.vel.x = a.vel.x + (b.vel.x - a.vel.x) * alpha;
    out.vel.y = a.vel.y + (b.vel.y - a.vel.y) * alpha;
    out.rotationDeg = a.rotationDeg + (b.rotationDeg - a.rotationDeg) * alpha;
    return out;
}

void MyLogger(U32 level, const char *consoleLine, void*)
{
    printf("%s\n", consoleLine);
}

namespace SimWorld
{

class ITickable
{
   struct TickableInfo
   {
      ITickable* tickable;
      bool registered;
   };
 
public:
   virtual void onFixedTick(F32 dt) = 0;
   
   void registerTickable()
   {
      TickableInfo info = {};
      info.tickable = this;
      info.registered = true;
      smTickList.push_back(info);
   }
   
   void unregisterTickable()
   {
      auto itr = std::find_if(smTickList.begin(), smTickList.end(), [this](const TickableInfo& info){
         return info.tickable == this;
      });
      
      if (itr != smTickList.end())
      {
         itr->registered = false;
      }
   }
   
   static void doFixedTick(F32 dt)
   {
      for (TickableInfo info : smTickList)
      {
         info.tickable->onFixedTick(dt);
      }
      
      smTickList.erase(
          std::remove_if(smTickList.begin(), smTickList.end(),
              [](const TickableInfo& info) {
                  return !info.registered;
              }),
                       smTickList.end()
      );
   }
   
   static std::vector<TickableInfo> smTickList;
};

std::vector<ITickable::TickableInfo> ITickable::smTickList;

typedef enum {
    UI_EVENT_MOUSE_MOVE,
    UI_EVENT_MOUSE_DOWN,
    UI_EVENT_MOUSE_UP,
    UI_EVENT_MOUSE_WHEEL,
    UI_EVENT_KEY_DOWN,
    UI_EVENT_KEY_UP,
    UI_EVENT_CHAR
} DBIEventType;

struct DBIEvent
{
    DBIEventType type;
   
   struct MouseData
   {
      Vector2 pos;
      S32 button;
      F32 wheelPos;
   };
   
   union
   {
      U32 key;
      U32 codePoint;
   };
   
};

class DisplayBase : public SimGroup
{
   typedef SimGroup Parent;
public:
   Point2I mPosition;
   Point2I mExtent;
   
   // NOTE: these are basically the style
   ColorI mBackColor;
   ColorI mColor;
   ColorI mHiColor;
   ColorI mDimColor;
   
   bool mCentered;
   bool mEnabled;
   U32 mHotKey;
   
   inline Point2I getPosition() const { return mPosition; }
   inline Point2I getExtent() const { return mExtent; }
   
   bool onAdd() override
   {
      return Parent::onAdd();
   }
   
   void onRemove() override
   {
      Parent::onRemove();
   }
   
   virtual void resize(const Point2I newPosition, const Point2I newExtent)
   {
      mPosition = newPosition;
      mExtent = newExtent;
      
      for (SimObject* obj : objectList)
      {
         DisplayBase* displayObj = dynamic_cast<DisplayBase*>(obj);
         if (displayObj)
         {
            displayObj->resize(newPosition, newExtent);
         }
      }
   }
   
   void forwardEvent(DBIEvent& event)
   {
      for (SimObject* obj : objectList)
      {
         DisplayBase* displayObj = dynamic_cast<DisplayBase*>(obj);
         if (displayObj)
         {
            displayObj->processInput(event);
         }
      }
   }
   
   virtual bool processInput(DBIEvent& event)
   {
      return false;
   }
   
   void renderChildren(Point2I offset, RectI drawRect, Camera2D& globalCamera)
   {
      for (SimObject* obj : objectList)
      {
         DisplayBase* dObj = dynamic_cast<DisplayBase*>(obj);
         if (dObj)
         {
            Point2I childPosition = offset + dObj->getPosition();
            RectI childClip(childPosition, dObj->getExtent());
            
            dObj->onRender(offset, drawRect, globalCamera);
         }
      }
   }
   
   virtual void onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera)
   {
   }
   
   
public:
   DECLARE_CONOBJECT(DisplayBase);
};

IMPLEMENT_CONOBJECT(DisplayBase);

class ImageSet;

class CostumeAnim : public SimObject
{
   typedef SimObject Parent;
   
   struct LimbControl
   {
      ImageSet* set;
      U32 frameNumber;
      U32 setFlags;
      StringTableEntry limbName;
      LimbControl* next;
   };
   
   LimbControl* mRootLookups[CostumeRenderer::NumDirections];
   
public:
   DECLARE_CONOBJECT(CostumeAnim);
};

IMPLEMENT_CONOBJECT(CostumeAnim);

enum Direction : U8
{
   NORTH,
   SOUTH,
   WEST,
   EAST
};

class Palette;

class Costume : public SimObject
{
   typedef SimObject Parent;
   
public:
   Palette* mPalette;
   std::vector<StringTableEntry> mLimbNames;
   U32 mCostumeFlags;
   
   CostumeRenderer::StaticState mState;
   
   void enumerateItems(std::vector<CostumeAnim*> &anims);
   bool compileCostume();
   
   static void initPersistFields()
   {
      Parent::initPersistFields();
      
      //addField("N", TypeString, )
      
   }
   
public:
   DECLARE_CONOBJECT(Costume);
};

IMPLEMENT_CONOBJECT(Costume);

bool Costume::compileCostume()
{
   mState.reset();
}

class RoomObject : public SimObject
{
   typedef SimObject Parent;
public:
   
   struct ObjectState
   {
      Point2I mOffset;
      StringTableEntry mImageName;
      TextureHandle mTexture;
      StringTableEntry mZPlaneName[RoomRender::NumZPlanes];
      TextureHandle mZPlaneTextures[RoomRender::NumZPlanes];
   };
   
   StringTableEntry mDescription;
   StringTableEntry mClassName;
   
   Point2I mPosition;
   Point2I mExtent;
   Point2I mHotspot;
   SimWorld::Direction mDirection;
   U32 mCurrentState;
   U32 mTransFlags;
   
   StringTableEntry mParentName;
   U32 mParentState;
   
   bool mActive;
   
   std::vector<ObjectState> mStates;
   
public:
   DECLARE_CONOBJECT(RoomObject);
};

IMPLEMENT_CONOBJECT(RoomObject);

class Charset : public SimObject
{
   typedef SimObject Parent;
public:
   StringTableEntry mPath;
   
public:
   DECLARE_CONOBJECT(Charset);
};

IMPLEMENT_CONOBJECT(Charset);

class ImageSet : public SimObject
{
   typedef SimObject Parent;
public:
   
   StringTableEntry mFormatString;
   Point2I mOffset;
   std::vector<Image> mLoadedImages;
   
   DECLARE_CONOBJECT(ImageSet);
};

IMPLEMENT_CONOBJECT(ImageSet);

class Palette : public SimObject
{
   typedef SimObject Parent;
public:
   
   StringTableEntry mFormatString;
   Image mImageData;
   
public:
   DECLARE_CONOBJECT(Palette);
};

IMPLEMENT_CONOBJECT(Palette);

class Actor : public SimObject
{
   typedef SimObject Parent;
public:
   
   SimWorld::Costume* mCostume;
   CostumeRenderer::LiveState mLiveCostume;
   
   
   DECLARE_CONOBJECT(Actor);
};

IMPLEMENT_CONOBJECT(Actor);

class VerbDisplay : public DisplayBase
{
   typedef DisplayBase Parent;
public:
   
   StringTableEntry mDisplayText;
   StringTableEntry mVerbName; // verb to set
   
public:
   DECLARE_CONOBJECT(VerbDisplay);
};

IMPLEMENT_CONOBJECT(VerbDisplay);

class ContainerDisplay : public DisplayBase
{
   typedef DisplayBase Parent;
public:
   DECLARE_CONOBJECT(ContainerDisplay);
};

IMPLEMENT_CONOBJECT(ContainerDisplay);

class RootUI : public DisplayBase
{
   typedef DisplayBase Parent;
public:
   static RootUI* sMainInstance;
   
   bool onAdd()
   {
      if (Parent::onAdd())
      {
         sMainInstance = this;
         return true;
      }
      return false;
   }
   
   void onRemove()
   {
      if (sMainInstance == this)
      {
         sMainInstance = nullptr;
      }
      
      Parent::onRemove();
   }
   
   virtual void onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera)
   {
      resize(offset, drawRect.extent);
      
      for (SimObject* obj : objectList)
      {
         DisplayBase* displayObj = dynamic_cast<DisplayBase*>(obj);
         if (displayObj)
         {
            displayObj->onRender(offset, drawRect, globalCamera);
         }
      }
   }
   
public:
   DECLARE_CONOBJECT(RootUI);
};
RootUI* RootUI::sMainInstance;

IMPLEMENT_CONOBJECT(RootUI);

class Sound : public SimObject
{
   typedef SimObject Parent;
public:
   
   StringTableEntry mPath;
   ::Sound mSound;
   
public:
   DECLARE_CONOBJECT(Sound);
};

IMPLEMENT_CONOBJECT(Sound);


struct BoxInfo
{
   enum
   {
      NumScaleSlots = 5
   };
   
   enum BoxFlags
   {
      BOXF_NONE = 0,
      
      BOXF_DISABLED = BIT(0),
      BOXF_UNKNOWN1 = BIT(1),
      BOXF_UNKNOWN2 = BIT(2),
      
      BOXF_XFLIP = BIT(3),
      BOXF_YFLIP = BIT(4),
      BOXF_IGNORE_SCALE = BIT(5),
      BOXF_LOCKED = BIT(6),
      BOXF_INVISIBLE = BIT(7)
   };
   
   struct Box
   {
      std::string name;
      uint16_t startPoint;
      uint16_t numPoints;
      uint16_t scale;
      uint8_t mask;
      uint8_t flags;
   };
   
   struct ScaleInfo
   {
      U16 sy1[2];
      U16 sy2[2];
   };
   
   std::vector<Box> boxes;
   std::vector<Point2I> points;
   std::vector<ScaleInfo> scaleBands;
   std::vector<U8> nextBoxHopList;

   static inline bool OnSegment(const Point2I& a, const Point2I& b, const Point2I& p)
   {
       return std::min(a.x, b.x) <= p.x && p.x <= std::max(a.x, b.x) &&
              std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y);
   }

   static inline bool PointOnEdge(Point2I* poly, U32 n, Point2I p)
   {
       for (int i = 0; i < n; i++) {
           const Point2I& a = poly[i];
           const Point2I& b = poly[(i + 1) % n];
           S32 c = mCross(a, b, p);
           if (c == 0 && OnSegment(a, b, p)) return true;
       }
       return false;
   }
   
   static bool PointInConvexPoly(Point2I* poly, U32 n, Point2I p)
   {
       if (n < 3) return false;

       if (PointOnEdge(poly, n, p)) return true;

       int sign = 0;
       for (int i = 0; i < n; i++) {
           const Point2I& a = poly[i];
           const Point2I& b = poly[(i + 1) % n];
           S32 c = mCross(a, b, p);
           if (c == 0) continue;
           int s = (c > 0) ? 1 : -1;
           if (sign == 0) sign = s;
           else if (sign != s) return false;
       }
       return true;
   }
   
   F32 evalScale(Point2I roomPos)
   {
      F32 outScale = 1.0f;
      U16 realRoomPos = std::clamp(roomPos.x, 0, 65536);
      evalBandScale(realRoomPos, outScale);
      evalBoxScale(roomPos, outScale); // may override
      return outScale;
   }
   
   void evalBoxScale(Point2I roomPos, F32& outScale)
   {
      for (Box& box : boxes)
      {
         if (PointInConvexPoly(points.data() + box.startPoint, box.numPoints, roomPos))
         {
            if (box.flags & BOXF_DISABLED)
            {
               continue;
            }
            
            if (box.flags & BOXF_IGNORE_SCALE)
            {
               outScale = 1.0;
            }
            else if ((box.scale & 0x8000) != 0)
            {
               U16 trueScale = box.scale & 0x7;
               U16 realRoomPos = std::clamp(roomPos.x, 0, 65536);
               outScale = evalScaleForBand(scaleBands[trueScale], realRoomPos);
            }
            else if (box.scale > 0)
            {
               outScale = scaleToFloat(box.scale);
            }
            
            return;
         }
      }
   }
   
   inline F32 evalScaleForBand(ScaleInfo& info, S16 roomPos)
   {
      F32 startScale = scaleToFloat(info.sy1[0]);
      F32 endScale = scaleToFloat(info.sy2[0]);
      
      U16 relPos = roomPos - info.sy1[1];
      U16 bandSize = info.sy2[1] - info.sy1[1];
      
      F32 ratioInBand = (F32)(relPos) / (F32) bandSize;
      return startScale + (ratioInBand * (endScale - startScale));
   }
   
   F32 evalBandScale(U16 roomPos, F32 outScale)
   {
      if (scaleBands.empty())
      {
         return 1.0f;
      }
      
      for (ScaleInfo& info : scaleBands)
      {
         if (roomPos >= info.sy1[1] && roomPos < info.sy2[1])
         {
            return evalScaleForBand(info, roomPos);
         }
      }
      
      return 1.0f;
   }
   
   uint8_t getNextBoxIndex(int src, int dst)
   {
       return nextBoxHopList[src * boxes.size() + dst];
   }
   
   static F32 scaleToFloat(uint16_t value)
   {
      return value / 100.0f;
   }
   
   void reset()
   {
      boxes.clear();
      points.clear();
      scaleBands.clear();
      nextBoxHopList.clear();
   }
   
   bool read(Stream& stream)
   {
      std::vector<U8> boxmData;
      
      while (stream.getStatus() == Stream::Ok)
      {
         IFFBlock block;
         if (!stream.read(sizeof(IFFBlock), &block))
         {
            return false;
         }
         
         S32 size = convertBEndianToHost(block.getRawSize());
         if (size < 8)
         {
            return false;
         }
         size -= 8;
         
         if (block.ident == 1297633090) // BOXM
         {
            boxmData.resize(size);
            if (!stream.read(size, boxmData.data()))
            {
               return false;
            }
         }
         else if (block.ident == 1685614434 || // boxd
                  block.ident == 1112496196)  // BOXD
         {
            // skip first box on BOXD
            if (block.ident == 1112496196)
            {
               stream.setPosition(stream.getPosition()+22);
            }
            
            if (stream.getStatus() == Stream::Ok)
            {
               char nameBuf[256];
               Box rootBox = {};
               boxes.push_back(rootBox);
               
               while (size >= 21)
               {
                  Box outBox = {};
                  U8 sz = 0;
                  stream.read(&sz);
                  stream.read(sz, nameBuf);
                  nameBuf[sz] = '\0';
                  outBox.name = nameBuf;
                  size -= sz + 1;
                  
                  Point2I point;
                  outBox.startPoint = points.size();
                  outBox.numPoints = 4;
                  
                  for (U32 i=0; i<4; i++)
                  {
                     uint16_t xp;
                     uint16_t yp;
                     stream.read(&xp);
                     stream.read(&yp);
                     points.push_back(Point2I(xp, yp));
                  }
                  
                  stream.read(&outBox.mask);
                  stream.read(&outBox.flags);
                  stream.read(&outBox.scale);
                  size -= 20;
                  
                  boxes.push_back(outBox);
               }
            }
         }
         else if (block.ident == 1279345491) // SCAL
         {
            scaleBands.clear();
            
            for (U32 i=0; i<NumScaleSlots; i++)
            {
               ScaleInfo info;
               stream.read(&info.sy1[0]);
               stream.read(&info.sy1[1]);
               stream.read(&info.sy2[0]);
               stream.read(&info.sy2[1]);
               scaleBands.push_back(info);
            }
         }
         else
         {
            return false;
         }
      }
      
      // Decode BOXM
      nextBoxHopList.clear();
      if (!boxmData.empty())
      {
         U32 matrixN = boxes.size();
         nextBoxHopList.resize(matrixN*matrixN, 255);
         
         MemStream boxStream(boxmData.size(), boxmData.data(), true, false);
         
         for (int row = 0; row < boxes.size(); row++)
         {
            uint8_t marker = 0;
            boxStream.read(&marker);
             if (marker != 0xFF)
             {
                return false;
             }

             // Runs continue until the next 0xFF, which begins the next row (or ends the block)
             while (boxStream.getStatus() == Stream::Ok)
             {
                uint8_t startCol = 0;
                uint8_t endCol   = 0;
                uint8_t value    = 0;
                
                 boxStream.read(&startCol);
                 if (startCol == 0xFF)
                 {
                    boxStream.setPosition(boxStream.getPosition()-1);
                    break;
                 }
                
                boxStream.read(&endCol);
                boxStream.read(&value);

                 if (startCol >= matrixN ||
                     endCol > matrixN ||
                     startCol > endCol)
                 {
                    return false;
                 }

                 for (int col = startCol; col <= endCol; col++)
                 {
                    nextBoxHopList[(size_t)row * matrixN + (size_t)col] = value;
                 }
             }
         }
      }
   }
};

RectI WorldRectToScreen(RectI r, Camera2D cam)
{
    Vector2 topLeft     = GetWorldToScreen2D((Vector2){ (float)(r.point.x), (float)(r.point.y) }, cam);
    Vector2 bottomRight = GetWorldToScreen2D((Vector2){ (float)(r.point.x + r.extent.x), (float)(r.point.y + r.extent.y) }, cam);

    return RectI(
        topLeft.x,
        topLeft.y,
        bottomRight.x - topLeft.x,
        bottomRight.y - topLeft.y
    );
}

class Room : public DisplayBase, public ITickable
{
   typedef DisplayBase Parent;
public:

    StringTableEntry mImageFileName;
    StringTableEntry mBoxFileName;
    StringTableEntry mZPlaneFiles[RoomRender::NumZPlanes];

    U32 mTransFlags;
   
   RoomRender mRenderState;
   BoxInfo mBoxes;
   
   bool onAdd()
   {
      if (Parent::onAdd())
      {
         registerTickable();
         return true;
      }
      return false;
   }
   
   void onRemove()
   {
      unregisterTickable();
   }
   
   void setTransitionMode(U8 mode, U8 param, F32 time)
   {
      mRenderState.transitionPos = 0.0f;
      mRenderState.transitionTime = time;
      mRenderState.currentTransition.mode = mode;
      mRenderState.currentTransition.params = param;
   }
   
   virtual void resize(const Point2I newPosition, const Point2I newExtent)
   {
      Parent::resize(newPosition, newExtent);
      mRenderState.screenSize.x = mExtent.x;
      mRenderState.screenSize.y = mExtent.y;
   }
   
   void updateResources()
   {
      if (mBoxFileName && mBoxFileName[0] != '\0')
      {
         mBoxes.reset();
         FileStream fs;
         if (fs.open(mBoxFileName, FileStream::Read))
         {
            mBoxes.read(fs);
         }
      }
      
      mRenderState.backgroundImage = mImageFileName && *mImageFileName ? gTextureManager->loadTexture(mImageFileName) : nullptr;
      for (uint32_t i=0; i<RoomRender::NumZPlanes; i++)
      {
         mRenderState.zPlanes[i] = mZPlaneFiles[i] && *mZPlaneFiles[i] ? gTextureManager->loadTexture(mZPlaneFiles[i]) : nullptr;
      }
      
      TextureSlot* slot = gTextureManager->resolveHandle(mRenderState.backgroundImage);
      if (slot)
      {
         mRenderState.backgroundSR = RectI(Point2I(0,0), Point2I(slot->mTexture.width, slot->mTexture.height));
      }
   }
   
   virtual void onRender(Point2I offset, RectI drawRect, Camera2D& globalCam)
   {
      if (mRenderState.backgroundImage.getNum() == 0)
      {
         updateResources();
      }
      
      // We need to maintain the original aspect ratio of the scene when drawing the image in respect to the scumm coord system
      
      Rectangle source = { (float)mRenderState.backgroundSR.point.x, (float)mRenderState.backgroundSR.point.y, (float)mRenderState.backgroundSR.extent.x, (float)mRenderState.backgroundSR.extent.y };
      Rectangle dest = { (float)offset.x, (float)offset.y, (float)drawRect.extent.x, (float)drawRect.extent.y };
      Vector2 origin = { 0.0, 0.0 };
      
      DrawRectangle(offset.x, offset.y, drawRect.extent.x, drawRect.extent.y, (Color){0,0,0,255});
      
      // Transform
      RectI globalRect = WorldRectToScreen(mRenderState.clipRect, globalCam);
      BeginScissorMode(globalRect.point.x, globalRect.point.y, globalRect.extent.x, globalRect.extent.y);
      
      TextureSlot* slot = gTextureManager->resolveHandle(mRenderState.backgroundImage);
      if (slot)
      {
         Rectangle dest = { (float)offset.x, (float)offset.y, (float)slot->mTexture.width, (float)slot->mTexture.height };
         DrawTexturePro(slot->mTexture, source, dest, origin, 0.0f, WHITE);
      }
      
      Point2F scalingFactor = Point2F(mRenderState.screenSize.x / 320.0f, mRenderState.screenSize.y / 200.0f);
      
      for (BoxInfo::Box& box : mBoxes.boxes)
      {
         if (box.numPoints >= 4)
         {
            Point2I* points = mBoxes.points.data() + box.startPoint;
            
            Vector2 prevPoint = (Vector2){points[0].x * scalingFactor.x, points[0].y * scalingFactor.y};
            Vector2 originPoint = prevPoint;
            for (U32 i=1; i<box.numPoints; i++)
            {
               Vector2 curPoint = (Vector2){points[i].x * scalingFactor.x, points[i].y * scalingFactor.y};
               ::DrawLineV(prevPoint, curPoint, (Color){255,255,255,255});
               prevPoint = curPoint;
            }
            
            ::DrawLineV(prevPoint, originPoint, (Color){255,255,255,255});
         }
      }
      
      EndScissorMode();
      
   }
   
   
   virtual void onFixedTick(F32 dt)
   {
      mRenderState.updateTransition(dt);
   }
   
   static void initPersistFields()
   {
      Parent::initPersistFields();
      
      addField("image", TypeString, Offset(mImageFileName, Room));
      addField("boxFile", TypeString, Offset(mBoxFileName, Room));
      addField("zPlanes", TypeString, Offset(mImageFileName, Room), RoomRender::NumZPlanes);
   }

public:
   DECLARE_CONOBJECT(Room);
};
IMPLEMENT_CONOBJECT(Room);


ConsoleMethodValue(RootUI, setContent, 3, 3, "")
{
   DisplayBase* displayObject = nullptr;
   if (Sim::findObject(argv[2], displayObject))
   {
      object->addObject(displayObject);
   }
   
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Room, setTransitionMode, 5, 5, "mode, param, time")
{
   object->setTransitionMode(vmPtr->valueAsInt(argv[2]), vmPtr->valueAsInt(argv[3]), vmPtr->valueAsFloat(argv[4]));
   return KorkApi::ConsoleValue();
}

};



ConsoleFunctionValue(IsKeyDown, 2, 2, "key")
{
   return KorkApi::ConsoleValue::makeUnsigned(IsKeyDown(vmPtr->valueAsInt(argv[1])));
}

Input gInput = { 0 };

ConsoleFunctionValue(SetInput, 3, 3, "")
{
   gInput.moveX = vmPtr->valueAsInt(argv[1]);
   gInput.moveY = vmPtr->valueAsInt(argv[2]);
   return KorkApi::ConsoleValue();
}


static Rectangle GetLetterboxViewport(int sw, int sh, int vw, int vh)
{
    float scale = fminf((float)sw/vw, (float)sh/vh);
    float w = vw * scale;
    float h = vh * scale;
    float x = (sw - w) * 0.5f;
    float y = (sh - h) * 0.5f;
    return (Rectangle){ x, y, w, h };
}

int main(int argc, char **argv)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    Con::init();
    Sim::init();
    Con::addConsumer(MyLogger, nullptr);
   ClearWindowState(FLAG_VSYNC_HINT);

   Camera2D cam = {0};
   cam.target = (Vector2){ 0, 0 };   // world origin you want at top-left
   cam.rotation = 0.0f;
   
    InitWindow(screenWidth, screenHeight, "raylib - fixed tick sim + variable render");
    {
       ClearWindowState(FLAG_VSYNC_HINT);
         SetTargetFPS(1000); // render as fast as possible (optional)
       
       Rectangle vp = GetLetterboxViewport(screenWidth, screenHeight, 320, 200);
       float zoom = vp.width / (float)320.0;   // same as vp.height/VH

       cam.offset = (Vector2){ vp.x, vp.y }; // screen-space top-left of viewport
       cam.zoom   = zoom;

        gTextureManager = new TextureManager();

        // Boot
        Con::executef(2, "exec", "boot.cs");

        TextureHandle rectTex = gTextureManager->loadTexture("graphics/original_art/back01.bmp");
        const Texture2D& texture = rectTex.getPtr()->mTexture;

        // Rectangle draw setup
        Vector2 size = { (float)texture.width, (float)texture.height };
        Vector2 origin = { size.x / 2.0f, size.y / 2.0f };

        const float fixedDt = 1.0f / (float)TICK_HZ;
        double accumulator = fixedDt;

        State prev = { .pos = { screenWidth/2.0f, screenHeight/2.0f }, .vel = {0}, .rotationDeg = 0.0f };
        State curr = prev;

        Rectangle source = { size.x * 0.25f, size.y * 0.25f, size.x * 0.5f, size.y * 0.5f };
        bool textBoxEditMode = false;
       char textBoxText[128] = {};

        while (!WindowShouldClose())
        {
            // Real frame delta time (variable)
            float frameDt = GetFrameTime();
            if (frameDt > (float)MAX_FRAME_DT) frameDt = (float)MAX_FRAME_DT;

            accumulator += frameDt;

            // Sample input once per render frame (common in Source-ish setups)
           auto start = std::chrono::steady_clock::now();
            Con::executef(1, "evalInput");
           
            Input in = gInput;//SampleInput();
           auto end = std::chrono::steady_clock::now();
           
           auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
           //Con::printf("Time: %u ms", duration.count());
           
           //exit(1);
           
            if (SimWorld::RootUI::sMainInstance)
            {
               SimWorld::RootUI::sMainInstance->resize(Point2I(0,0), Point2I(320, 200));
            }

            // Run fixed sim steps as needed
            int steps = 0;
            while (accumulator >= fixedDt && steps < MAX_STEPS)
            {
                prev = curr;                 // keep last state for interpolation
                SimWorld::ITickable::doFixedTick(fixedDt);
                UpdateFixed(&curr, in, fixedDt);
                accumulator -= fixedDt;
                steps++;
            }

            // Interpolation factor for smooth rendering
            float alpha = (float)(accumulator / fixedDt);
            State renderState = LerpState(prev, curr, alpha);

            BeginDrawing();
            ClearBackground(RAYWHITE);
           BeginMode2D(cam);
           
            if (SimWorld::RootUI::sMainInstance)
            {
               SimWorld::RootUI::sMainInstance->onRender(Point2I(0,0), RectI(Point2I(0,0), Point2I(320, 200)), cam);
            }
           
           
           if (GuiButton((Rectangle){ 24, 24, 120, 30 }, "#191#Show Message"))
           {
              Con::printf("message");
           }
           
           if (GuiTextBox((Rectangle){ 25, 215, 125, 30 }, textBoxText, 64, textBoxEditMode))
           {
              textBoxEditMode = !textBoxEditMode;
           }

            Rectangle dest = { renderState.pos.x, renderState.pos.y, size.x, size.y };
            DrawTexturePro(texture, source, dest, origin, renderState.rotationDeg, WHITE);

            DrawText(TextFormat("Sim tick: %.0f Hz (dt=%.6f) FPS=%i", TICK_HZ, fixedDt, GetFPS()), 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("alpha=%.2f steps=%d", alpha, steps), 10, 35, 20, DARKGRAY);
            DrawText("Move: WASD/Arrows | Fixed sim, variable render", 10, 60, 20, DARKGRAY);

           EndMode2D();
           
           // Debug viewport outline
           DrawRectangleLinesEx(vp, 1, GREEN);

            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}
