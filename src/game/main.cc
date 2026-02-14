//-----------------------------------------------------------------------------
// Copyright (c) 2025-2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// 
#include "platform/platform.h"
#include "platform/platformProcess.h"
#include "raylib.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "math/mathTypes.h"
#include "sim/simBase.h"
#include "core/freeListHandleHelpers.h"
#include "core/memStream.h"
#include "core/fileStream.h"
#include "core/stringUnit.h"
#include <math.h>
#include <sstream>
#include <iomanip>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "sim/simFiberManager.h"

// --- Tweak these ---
#define TICK_HZ        60.0        // fixed simulation rate (e.g. 60, 64, 128)
#define MAX_FRAME_DT   0.25         // clamp to avoid spiral-of-death (seconds)
#define MAX_STEPS      8            // safety cap: max sim steps per render frame

#define PINK_BG (Color){253, 5, 255, 255}

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
      if (newTO && other.isHeavyRef())
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

    TextureHandle loadTexture(const std::string& path, Image* existingImage = NULL)
    {
        // Already loaded?
        auto it = mPathToId.find(path);
        if (it != mPathToId.end()) {
            U32 id = it->second;
            return TextureHandle(id);
        }

        // Load new texture
        ::Texture2D tex = existingImage ? ::LoadTextureFromImage(*existingImage) : ::LoadTexture(path.c_str());
       
        if (tex.id == 0)
        {
           Con::errorf("Failed to load image '%s'", path.c_str());
           return TextureHandle();
        }

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


// globals
F32 gTimerNext = 1.0;
SimFiberManager* gFiberManager = nullptr;
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

namespace SimWorld
{
   class Sound;
}

struct CostumeRenderer
{
    enum AnimFlags
    {
        LOOP=0,
        NO_LOOP=BIT(1),
        HIDE=BIT(3)
    };
   
    enum GlobalFlags
   {
       FLIP=BIT(0)
    };
   
    enum CommandOpcode : U16
    {
       CMD_IMG,    // change image
       CMD_SOUND,  // play sound
       CMD_HIDE,   // hide image
       CMD_SHOW,   // show image
       CMD_NOP,    // do nothing
       CMD_COUNT,  // movement step for walk
       CMD_FLAG,   // flag set (not used at runtime)
       CMD_END
    };
   
   static const char* opcodeMap[];

    enum
    {
        NumDirections = 4
    };
   
   enum DirectionValue : U8
   {
      NORTH,
      SOUTH,
      WEST,
      EAST
   };
   
   /*
    Layout:
    
       LimbState[numLimbs]
       For each anim:
         AnimInfo
            For each direction:
               AnimDirection
                  AnimLimbMap[]
                     LimbTrack
                        Command[]
       Frame[numFrames]
    */

    // Limb track for direction
    struct AnimDirection
    {
        U32 startLimbMap; // index into AnimLimbMap
        U32 numLimbs;     // x NumDirections
        U32 flags;        // global anim flags; loop, etc
    };

    struct AnimInfo
    {
        AnimDirection directionTracks[NumDirections]; // tracks for each dir
        StringTableEntry name;
        U32 flags; // loop, etc
    };
   
   struct LimbTrack
   {
      U32 startCmd;        // start frame of current loop
      U32 numCommands;     // 0 = invisible
      U32 flags;           // limb specific anim flags
   };

    struct AnimLimbMap
    {
       LimbTrack track;
       U32 targetLimb;   // limb the track is for
    };

    // compiled frame
    struct Frame
    {
        TextureHandle displayImage;
        Point2I displayOffset;
       U8 setFlags;
    };
   
    // compiled command
    struct Command
    {
       U16 cmd;
       U16 param;
    };

    // State of limbs
    struct LimbState
    {
        StringTableEntry name;
        LimbTrack track;
        U32 nextCmd;         // command we are on next
        U32 lastEvalFrame;   // last Frame we are displaying
    };

    // compiled state
    struct StaticState
    {
        std::vector<Frame> mFrames;
        std::vector<Command> mCommands;
        std::vector<AnimLimbMap> mLimbMap;
        std::vector<AnimInfo> mAnims;
        std::vector<StringTableEntry> mLimbNames; // base names for LimbState
        std::vector<SimWorld::Sound*> mSounds;
        U32 mFlags;
       
       void reset(bool arraysOnly=false)
       {
          mFrames.clear();
          mCommands.clear();
          mLimbMap.clear();
          mAnims.clear();
          mLimbNames.clear();
          if (!arraysOnly)
          {
             mFlags = 0;
          }
       }
    };

    // live state
    struct LiveState
    {
        U8 globalFlags;   // baked costume flags
        U8 animFlags;     // this is global ANIM flags
        S16 curAnim;      // AnimInfo we are playing
        S16 curTalkAnim;  // AnimInfo for talk (override)
        U8 curDirection;  // current direction
        U16 curCount;     // walk counter (based on current movement direction)
        Point2F position; // display position
        Point2F delta;    // momentum
        F32 scale;
        std::vector<LimbState> mLimbState;

       void init(StaticState& state)
       {
          reset();
         mLimbState.resize(state.mLimbNames.size());
         for (U32 i=0; i<mLimbState.size(); i++)
         {
            mLimbState[i].name = state.mLimbNames[i];
            mLimbState[i].track = {};
            mLimbState[i].nextCmd = 0;
            mLimbState[i].lastEvalFrame = 0;
         }
          
          globalFlags = (U8)state.mFlags;
       }

       void reset()
       {
          globalFlags = 0;
          animFlags = 0;
          curAnim = 0;
          curTalkAnim = -1;
          curDirection = 0;
          position = Point2F(0.0f, 0.0f);
          mLimbState.clear();
          position = Point2F(0.0f, 0.0f);
          delta = Point2F(0.0f, 0.0f);
          scale = 1.0f;
       }
       
       void resetAnim(StaticState& state, U8 direction, bool talking)
       {
          S16 animNumber = talking ? curTalkAnim : curAnim;
          if (animNumber < 0)
          {
             return;
          }
          
          AnimInfo& animInfo = state.mAnims[animNumber];
          AnimDirection& dirInfo = animInfo.directionTracks[direction];
          
          for (U32 k=0; k<dirInfo.numLimbs; k++)
          {
             AnimLimbMap& limbTrack = state.mLimbMap[dirInfo.startLimbMap + k];
             LimbState& liveLimb = mLimbState[limbTrack.targetLimb];
             liveLimb.track = limbTrack.track;
             liveLimb.nextCmd = 0;
          }
       }

       void setAnim(StaticState& state, StringTableEntry animName, U8 direction, bool talking)
       {
         for (U32 i=0; i<state.mAnims.size(); i++)
         {
            AnimInfo& animInfo = state.mAnims[i];
            animFlags = animInfo.flags;

            if (animInfo.name == animName)
            {
               // Found it, reset to this anim
               AnimDirection& dirInfo = animInfo.directionTracks[direction];

               for (U32 k=0; k<dirInfo.numLimbs; k++)
               {
                  AnimLimbMap& limbTrack = state.mLimbMap[dirInfo.startLimbMap + k];
                  LimbState& liveLimb = mLimbState[limbTrack.targetLimb];
                  liveLimb.track = limbTrack.track;
                  liveLimb.nextCmd = 0;
               }

               // Track current anim
               if (talking)
               {
                  curTalkAnim = i;
               }
               else
               {
                  curAnim = i;
               }
               
               return;
            }
         }
       }

       inline void evalCmd(StaticState& state, LimbState& limbState, Command& cmd)
       {
         switch (cmd.cmd)
         {
         case CMD_IMG:
            limbState.lastEvalFrame = cmd.param;
            break;
         case CMD_HIDE:
            limbState.track.flags |= HIDE;
            break;
         case CMD_SHOW:
            limbState.track.flags &= ~HIDE;
            break;
         case CMD_COUNT:
            curCount++;
            break;
         case CMD_SOUND:
            // TODO
            break;
         default:
            break;
         }

          if (limbState.nextCmd+1 < limbState.track.numCommands)
          {
             limbState.nextCmd++;
          }
          else if ((limbState.track.flags & NO_LOOP) == 0)
          {
             // loop back to start
             limbState.nextCmd = 0;
          }
       }

       void advanceTick(StaticState& state)
       {
         for (LimbState& limbState : mLimbState)
         {
            if (limbState.nextCmd < limbState.track.numCommands)
            {
               evalCmd(state, limbState, state.mCommands[limbState.track.startCmd + limbState.nextCmd]);
            }
         }
       }

       void render(StaticState& state);
    };
};


const char* CostumeRenderer::opcodeMap[] = {
   "IMG",
   "SOUND",
   "HIDE",
   "SHOW",
   "NOP",
   "COUNT",
   "FLAG",
   ""
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


class ImageSet : public SimObject
{
   typedef SimObject Parent;
public:
   
   enum Flags
   {
      FLAG_TRANSPARENT = BIT(0)
   };
   
   StringTableEntry mFormatString;
   Point2I mOffset;
   std::vector<Image> mLoadedImages;
   U32 mFlags;
   
   ImageSet()
   {
      mFormatString = StringTable->insert("");
      mFlags = FLAG_TRANSPARENT;
      mOffset = Point2I(0,0);
   };
   
   ~ImageSet()
   {
      clearImages();
   }
   
   void clearImages()
   {
      for (Image& img : mLoadedImages)
      {
         ::UnloadImage(img);
      }
      mLoadedImages.clear();
   }
   
   void ensureImageLoaded(U32 n)
   {
      if (mLoadedImages.size() <= n)
      {
         for (U32 i=(U32)mLoadedImages.size(); i<=n; i++)
         {
            std::string fpath = makeImageFilename(n);
            char dstName[4096];
            Con::expandPath(dstName, sizeof(dstName), fpath.c_str(), Con::getCurrentCodeBlockFullPath());
            if (Platform::isFile(dstName))
            {
               Image img = ::LoadImage(dstName);
               if (img.data)
               {
                  if ((mFlags & FLAG_TRANSPARENT) != 0)
                  {
                     ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
                     ImageColorReplace(&img, PINK_BG, BLANK);
                  }
                  mLoadedImages.push_back(img);
               }
               else
               {
                  mLoadedImages.push_back(Image());
               }
            }
            else
            {
               mLoadedImages.push_back(Image());
            }
         }
      }
   }
   
   std::string makeImageFilename(U32 n)
   {
       std::ostringstream ss;
       ss << std::setw(2) << std::setfill('0') << n;

       std::string path = mFormatString;
       size_t pos = path.find("??");
       if (pos != std::string::npos) {
           path.replace(pos, 2, ss.str());
       }
       return path;
   }
   
   static void initPersistFields()
   {
      Parent::initPersistFields();
      
      addField("format", TypeString, Offset(mFormatString, ImageSet));
      addField("offset", TypePoint2I, Offset(mOffset, ImageSet));
      addField("flags", TypeS32, Offset(mFlags, ImageSet));
   }
   
   DECLARE_CONOBJECT(ImageSet);
};

IMPLEMENT_CONOBJECT(ImageSet);


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
            
            dObj->onRender(childPosition, childClip, globalCamera);
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

ConsoleMethodValue(DisplayBase, setPosition, 4, 4, "")
{
   object->mPosition = Point2I(vmPtr->valueAsInt(argv[2]), vmPtr->valueAsInt(argv[3]));
   return KorkApi::ConsoleValue();
}

class ImageSet;

DefineConsoleType(TypeLimbControlVector);


class CostumeAnim : public SimObject
{
   typedef SimObject Parent;
public:
   
   struct LimbControl
   {
      SimObjectId setId;
      U16 setCommand;
      U16 setParam;
   };
   
   struct LimbRoot
   {
      std::vector<LimbControl> entries;
      StringTableEntry limbName;
      LimbRoot* next;
      
      LimbRoot() : limbName(nullptr), next(nullptr) {;}
   };
   
   LimbRoot* mRootLookups[CostumeRenderer::NumDirections];
   U32 mAnimFlags;
   
   static bool getLimbStorage(KorkApi::Vm* vmPtr, void* obj, const KorkApi::FieldInfo* field, KorkApi::ConsoleValue arrayValue, KorkApi::TypeStorageInterface* outStorage, bool writeMode)
   {
      StringTableEntry steArray = StringTable->insert(vmPtr->valueAsString(arrayValue));
      
      LimbRoot* foundEntry = nullptr;
      LimbRoot** entryRoot = (LimbRoot**)(((uintptr_t)obj) + field->offset);
      for (LimbRoot* ctrl = *entryRoot; ctrl; ctrl = ctrl->next)
      {
         if (ctrl->limbName == steArray)
         {
            foundEntry = ctrl;
            break;
         }
      }
      
      // Alloc new entry
      if (foundEntry == NULL)
      {
         if (!writeMode)
         {
            return false;
         }
         
         foundEntry = new LimbRoot();
         foundEntry->next = *entryRoot;
         *entryRoot = foundEntry;
         foundEntry->limbName = steArray;
      }
      
      *outStorage = {};
      return vmPtr->initFixedTypeStorage(&foundEntry->entries, TypeLimbControlVector, true, outStorage);
   }
   
   static bool enumerateLimbStorage(void* user, KorkApi::Vm* vmPtr, KorkApi::VMObject* obj, const KorkApi::FieldInfo* field, KorkApi::FieldKeyVisitorFn visit)
   {
      // TODO: visit all root entries and enumerate
      // typedef bool (*FieldKeyVisitorFn)( void* user, KorkApi::Vm* vmPtr, KorkApi::VMObject* obj, KorkApi::ConsoleValue key, KorkApi::ConsoleValue value );
      //visit(field->fieldUserPtr, vmPtr, obj, KorkApi::ConsoleValue::makeString("N"), );
      return false;
   }
   
   //  bool (*WriteDataNotifyFn)( void* obj, StringTableEntry pFieldName );
   static bool shouldWriteLimb(void* obj, StringTableEntry pFieldName)
   {
      // TODO: should check if pFieldName entry has entries
      return true;
   }
   
   CostumeAnim()
   {
      memset(mRootLookups, 0, sizeof(mRootLookups));
      mAnimFlags = 0;
   }
   
   bool onAdd()
   {
      if (Parent::onAdd())
      {
         return true;
      }
      return false;
   }
   
   static void initPersistFields()
   {
      Parent::initPersistFields();
      
      addProtectedField("N", TypeLimbControlVector, Offset(mRootLookups[0], CostumeAnim), nullptr, getLimbStorage, enumerateLimbStorage, shouldWriteLimb);
      addProtectedField("S", TypeLimbControlVector, Offset(mRootLookups[1], CostumeAnim), nullptr, getLimbStorage, enumerateLimbStorage, shouldWriteLimb);
      addProtectedField("E", TypeLimbControlVector, Offset(mRootLookups[2], CostumeAnim), nullptr, getLimbStorage, enumerateLimbStorage, shouldWriteLimb);
      addProtectedField("W", TypeLimbControlVector, Offset(mRootLookups[3], CostumeAnim), nullptr, getLimbStorage, enumerateLimbStorage, shouldWriteLimb);
      
      addField("flags", TypeS32, Offset(mAnimFlags, CostumeAnim));
   }
   
public:
   DECLARE_CONOBJECT(CostumeAnim);
};

IMPLEMENT_CONOBJECT(CostumeAnim);

ConsoleType( limbControlVector, TypeLimbControlVector, sizeof(std::vector<CostumeAnim::LimbControl>), UINT_MAX, "" )
ConsoleTypeOpDefaultNumeric( TypeLimbControlVector )

static inline const char* SkipSpaces(const char* p)
{
   while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ++p;
   return p;
}



enum Direction : U8
{
   NORTH,
   SOUTH,
   WEST,
   EAST
};

class Palette;

class Costume : public SimGroup
{
   typedef SimObject Parent;
   
public:
   Palette* mPalette;
   std::vector<StringTableEntry> mLimbNames;
   
   CostumeRenderer::StaticState mState;
   
   void enumerateItems(std::vector<CostumeAnim*> &anims);
   bool compileCostume();
   
   Costume()
   {
      mPalette = NULL;
      mState.reset();
   }
   
   static void initPersistFields()
   {
      Parent::initPersistFields();
      
      addField("limbs", TypeStringTableEntryVector, Offset(mLimbNames, Costume), "");
      addField("flags", TypeS32, Offset(mState.mFlags, Costume), "");
      addField("palette", TypeSimObjectPtr, Offset(mPalette, Costume), "");
   }
   
public:
   DECLARE_CONOBJECT(Costume);
};

IMPLEMENT_CONOBJECT(Costume);

ConsoleMethodValue(Costume, compileCostume, 2, 2, "")
{
   return KorkApi::ConsoleValue::makeUnsigned(object->compileCostume());
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


bool Costume::compileCostume()
{
   mState.reset(true);

   Con::printf("Costume %s...", getName());
   
   if (strcasecmp(getName(), "ZifCostume") == 0)
   {
      Con::printf("ziffy");
   }
   
   mState.mLimbNames = mLimbNames;
   
   // Assemble limb names

   // Print out plan
   for (SimObject* obj : objectList)
   {
      CostumeAnim* anim = dynamic_cast<CostumeAnim*>(obj);
      if (anim)
      {
         Con::printf("-- Anim %s --", anim->getInternalName());
         
         CostumeRenderer::AnimInfo animInfo = {};
         animInfo.name = StringTable->insert(anim->getInternalName());
         
         for (U32 i=0; i<CostumeRenderer::NumDirections; i++)
         {
            CostumeRenderer::AnimDirection animDir = {};
            animDir.startLimbMap = (U32)mState.mLimbMap.size();
            animDir.flags = anim->mAnimFlags;
            
            for (CostumeAnim::LimbRoot* rootLimb = anim->mRootLookups[i];
               rootLimb;
               rootLimb = rootLimb->next)
            {
               Con::printf("Limb %s DIR: %i\n", rootLimb->limbName, i);
               
               auto itr = std::find(mLimbNames.begin(), mLimbNames.end(), rootLimb->limbName);
               if (itr == mLimbNames.end())
               {
                  Con::printf("Skipping (not in costume)");
                  continue;
               }
               
               U32 localIndex = (U32)(itr - mLimbNames.begin());
               
               CostumeRenderer::AnimLimbMap limbRoot = {};
               limbRoot.targetLimb = localIndex;
               limbRoot.track.startCmd = mState.mCommands.size();
               animDir.numLimbs++;
               
               // NOTE: flags are merged into the limb track
               
               CostumeRenderer::Frame buildFrame = {};
               
               for (CostumeAnim::LimbControl& ctrl : rootLimb->entries)
               {
                  CostumeRenderer::Command cmd = {};
                  cmd.cmd = ctrl.setCommand;
                  
                  if (ctrl.setCommand == CostumeRenderer::CMD_IMG)
                  {
                     // Find image in list
                     
                     ImageSet* theSet = NULL;
                     if (!Sim::findObject(ctrl.setId, theSet))
                     {
                        Con::errorf("Cant find ImageSet %i", ctrl.setId);
                        mState.reset();
                        return false;
                     }
                     
                     // set frame number
                     CostumeRenderer::Frame frame = {};
                     theSet->ensureImageLoaded(ctrl.setParam);
                      frame.displayImage = gTextureManager->loadTexture(theSet->makeImageFilename(ctrl.setParam),
                                                                        &theSet->mLoadedImages[ctrl.setParam]);
                      frame.displayOffset = theSet->mOffset;
                     frame.setFlags = (U8)theSet->mFlags;
                     cmd.param = mState.mFrames.size();
                     mState.mFrames.push_back(frame);
                     
                     if (frame.displayImage.getNum() == 0)
                     {
                        Con::errorf("Cant load image %i from set %s [%s]", ctrl.setParam, theSet->getInternalName(), theSet->makeImageFilename(ctrl.setParam).c_str());
                     }
                  }
                  else if (ctrl.setCommand == CostumeRenderer::CMD_SOUND)
                  {
                     // Find sound in list
                     
                     SimWorld::Sound* theSound = NULL;
                     if (!Sim::findObject(ctrl.setId, theSound))
                     {
                        Con::errorf("Cant find Sound %i", ctrl.setId);
                        mState.reset();
                        return false;
                     }
                     
                     auto itr = std::find(mState.mSounds.begin(), mState.mSounds.end(), theSound);
                     if (itr == mState.mSounds.end())
                     {
                        cmd.param = mState.mSounds.size();
                        mState.mSounds.push_back(theSound);
                     }
                     else
                     {
                        cmd.param = (U32)(itr - mState.mSounds.begin());
                     }
                  }
                  else if (ctrl.setCommand == CostumeRenderer::CMD_FLAG)
                  {
                     limbRoot.track.flags |= ctrl.setParam;
                     continue;
                  }
                  else
                  {
                     cmd.param = ctrl.setParam;
                  }
                  
                  
                  limbRoot.track.numCommands++;
                  mState.mCommands.push_back(cmd);
               }
               
               mState.mLimbMap.push_back(limbRoot);
            }
            
            animInfo.directionTracks[i] = animDir;
         }
         
         mState.mAnims.push_back(animInfo);
      }
   }
   
   Con::printf("Compile complete");

}

ConsoleMethodValue(ImageSet, pick, 3, 4, "(start, end)")
{
   S32 startValue = vmPtr->valueAsInt(argv[2]);
   S32 endValue = argc < 4 ? startValue : vmPtr->valueAsInt(argv[3]);
   
   if (!(endValue >= startValue && startValue >= 0 && endValue >= 0)
       && endValue < object->mLoadedImages.size())
   {
      return KorkApi::ConsoleValue();
   }
   
   std::vector<CostumeAnim::LimbControl> limbs;
   for (U32 i=startValue; i<=endValue; i++)
   {
      CostumeAnim::LimbControl limb = {};
      limb.setId = object->getId();
      limb.setCommand = CostumeRenderer::CMD_IMG;
      limb.setParam = i;
      limbs.push_back(limb);
   }
   
   // -> output as value
   KorkApi::ConsoleValue retValue = vmPtr->getTypeReturn(TypeLimbControlVector);
   
   KorkApi::TypeStorageInterface inputStorage = {};
   KorkApi::TypeStorageInterface outputStorage = {};
   
   vmPtr->initFixedTypeStorage(&limbs, TypeLimbControlVector, true, &inputStorage);
   vmPtr->initReturnTypeStorage(sizeof(CostumeAnim::LimbControl) * limbs.size() + sizeof(U32), TypeLimbControlVector, &outputStorage);
   vmPtr->castValue(TypeLimbControlVector, &inputStorage, &outputStorage, NULL, 0);
   return outputStorage.data.storageAddress;
}

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

class Actor;

struct ActorWalkState
{
   Point2I mWalkTarget;
   Point2I mWalkSpeed; // x,y units per tick
   CostumeRenderer::DirectionValue mDirection; // this is calculated direction
   bool mMoving;
   U32 mTieAxis;
   
   ActorWalkState() : mWalkTarget(0,0), mMoving(false), mWalkSpeed(0,0), mDirection(CostumeRenderer::SOUTH), mTieAxis(0)
   {
      mWalkSpeed = Point2I(2,1);
   }
   
   void reset()
   {
      mWalkTarget = Point2I(0,0);
      mWalkSpeed = Point2I(0,0);
      mDirection = CostumeRenderer::SOUTH;
      mMoving = false;
      mTieAxis = 0;
   }
   
   int sign(int x) {
       return (x > 0) - (x < 0);
   }
   
   S32 clampStep(S32 delta, S32 step)
   {
      return std::min<S32>(abs(delta), step) * sign(delta);
   }
   
   U32 pickAxis(Point2I delta)
   {
      U32 axis = 0;
      
      if (delta.x == 0)
      {
         axis = 1;
      }
      else if (delta.y == 0)
      {
         axis = 0;
      }
      else
      {
         if (delta.x > delta.y)
         {
            axis = 0;
         }
         else if (delta.y > delta.x)
         {
            axis = 1;
         }
         else
         {
            axis = mTieAxis;
         }
      }
      
      return axis;
   }
   
   CostumeRenderer::DirectionValue dirFromDominantAxis(S32 value, U32 axis)
   {
      static CostumeRenderer::DirectionValue lookupTable[] = {
         // x
         CostumeRenderer::WEST,
         CostumeRenderer::EAST,
         // y
         CostumeRenderer::NORTH,
         CostumeRenderer::SOUTH,
      };
      
      return lookupTable[(2 * axis) + (value > 0 ? 1 : 0)];
   }
   
   void updateTick(Actor& actor);
};

class Actor : public DisplayBase, public ITickable
{
   typedef DisplayBase Parent;
public:
   
   SimWorld::Costume* mCostume;
   CostumeRenderer::LiveState mLiveCostume;
   U16 mTickCounter;
   U16 mTickSpeed;
   
   ActorWalkState mWalkState;
   
   Actor()
   {
      mCostume = nullptr;
      mTickCounter = 0;
      mTickSpeed = 4;
   }
   
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
   
   void walkTo(Point2I pos)
   {
      mWalkState.mWalkTarget = pos;
      if (mPosition != pos)
      {
         mWalkState.mMoving = true;
         startAnim(StringTable->insert("walk"), false);
      }
   }
   
   virtual void onFixedTick(F32 dt)
   {
      if (mTickCounter == 0)
      {
         if (mCostume)
         {
            bool wasMoving = mWalkState.mMoving;
            mWalkState.updateTick(*this);
            if (mWalkState.mMoving != wasMoving)
            {
               startAnim(StringTable->insert("stand"), false);
            }
            mLiveCostume.advanceTick(mCostume->mState);
         }
      }
      
      mTickCounter++;
      if (mTickCounter >= mTickSpeed)
      {
         mTickCounter = 0;
      }
   }
   
   virtual void onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera)
   {
      if (mCostume)
      {
         mLiveCostume.position = Point2F(offset.x, offset.y);
         //mLiveCostume.w
         mLiveCostume.render(mCostume->mState);
      }
   }
   
   void startAnim(StringTableEntry animName, bool isTalking)
   {
      mLiveCostume.setAnim(mCostume->mState, animName, mLiveCostume.curDirection, false);
   }
   
   void setDirection(CostumeRenderer::DirectionValue direction)
   {
      mWalkState.mDirection = direction;
      if (mLiveCostume.curDirection != direction)
      {
         mLiveCostume.curDirection = direction;
         mLiveCostume.resetAnim(mCostume->mState, mLiveCostume.curDirection, false);
      }
   }

   void setCostume(SimWorld::Costume* costume)
   {
      if (costume != mCostume)
      {
         mLiveCostume.init(costume->mState);
         mCostume = costume;
         mLiveCostume.setAnim(costume->mState, StringTable->insert("stand"), 1, false);
         mTickCounter = 0;
      }
   }
   
   
   DECLARE_CONOBJECT(Actor);
};

IMPLEMENT_CONOBJECT(Actor);


void ActorWalkState::updateTick(Actor& actor)
{
   if (!mMoving)
   {
      return;
   }
   
   Point2I delta = mWalkTarget - actor.mPosition;
   
   if (delta == Point2I(0,0))
   {
      mMoving = false;
   }
   
   if (!mMoving)
   {
      return;
   }
   
   U32 axis = pickAxis(delta);
   CostumeRenderer::DirectionValue curDir = dirFromDominantAxis(axis == 0 ? delta.x : delta.y, axis);
   if (curDir != mDirection)
   {
      actor.setDirection(curDir);
   }
   
   // Apply movement
   if (axis == 0)
   {
      S32 step = clampStep(delta.x, mWalkSpeed.x);
      actor.mPosition.x += step;
   }
   else
   {
      S32 step = clampStep(delta.y, mWalkSpeed.y);
      actor.mPosition.y += step;
   }
}

ConsoleMethodValue(Actor, setCostume, 3, 3, "")
{
   SimWorld::Costume* cost = nullptr;
   if (Sim::findObject(argv[2], cost))
   {
      object->setCostume(cost);
   }
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, animate, 3, 3, "")
{
   object->startAnim(StringTable->insert(vmPtr->valueAsString(argv[2])), false);
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, waitFor, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getWidth, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, isInBox, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, isMoving, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, say, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, putAt, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setDirection, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, putAtObject, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, walkTo, 4, 4, "")
{
   object->walkTo(Point2I(vmPtr->valueAsInt(argv[2]), vmPtr->valueAsInt(argv[3])));
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, walkToObject, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, print, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getScale, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getLayer, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getFrame, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getAnimVar, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getAnimCounter, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setFrozen, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setTalkScript, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setStanding, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setIgnoreTurns, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setElevation, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setDefaultFrames, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setTalkColor, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setDescriptiveName, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setInitFrame, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setWidth, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setScale, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setZClip, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setIgnoreBoxes, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setAnimSpeed, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setTalkPos, 4, 4, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setAnimVar, 4, 4, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setLayer, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(yieldFiber, 2, 2, "value")
{
   vmPtr->suspendCurrentFiber();
   return argv[1]; // NOTE: this will be set as yield value
}

ConsoleFunctionValue(delayFiber, 2, 2, "ticks")
{
   SimFiberManager::ScheduleParam sp;
   sp.flagMask = 0;
   sp.minTime = gFiberManager->getCurrentTick() + vmPtr->valueAsInt(argv[1]);
   gFiberManager->setFiberWaitMode(vmPtr->getCurrentFiber(), SimFiberManager::WAIT_TICK, sp);
   vmPtr->suspendCurrentFiber();
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(spawnFiber, 2, 20, "func, ...")
{
   SimFiberManager::ScheduleInfo initialInfo = {};
   initialInfo.waitMode = SimFiberManager::WAIT_IGNORE;
   KorkApi::FiberId fiberId = gFiberManager->spawnFiber(NULL, argc-1, argv+1, initialInfo);
   
   if (vmPtr->getFiberState(fiberId) < KorkApi::FiberRunResult::State::ERROR)
   {
      return KorkApi::ConsoleValue::makeUnsigned(fiberId);
   }
   else
   {
      return KorkApi::ConsoleValue();
   }
}

ConsoleFunctionValue(throwFiber, 3, 3, "value, soft")
{
   vmPtr->throwFiber((vmPtr->valueAsInt(argv[1]) | vmPtr->valueAsInt(argv[2])) ? BIT(31) : 0);
}



ConsoleGetType( TypeLimbControlVector )
{
   std::vector<CostumeAnim::LimbControl>* vec = nullptr;
   static std::vector<CostumeAnim::LimbControl> workVec;

   auto parseLimb = +[](const char* word, CostumeAnim::LimbControl& outLimb){
      const char* paramName = StringUnit::getUnit(word, 0, ":");
      U32 unitCount = StringUnit::getUnitCount(word, ":");
      
      if (strcasecmp(paramName, "IMG") == 0)
      {
         // object:frame
         const char* objName = StringUnit::getUnit(word, 1, ":");
         ImageSet* imgSet = nullptr;
         if (!Sim::findObject(objName, imgSet))
         {
            return false;
         }

         outLimb.setId = imgSet->getId();
         outLimb.setCommand = CostumeRenderer::CMD_IMG;
         outLimb.setParam = (U16)atoi(StringUnit::getUnit(word, 2, ":"));
         return true;
      }
      else if (strcasecmp(paramName, "SOUND") == 0)
      {
         const char* objName = StringUnit::getUnit(word, 1, ":");
         SimWorld::Sound* sound = nullptr;
         if (!Sim::findObject(objName, sound))
         {
            return false;
         }
         
         outLimb.setId = sound->getId();
         outLimb.setCommand = CostumeRenderer::CMD_SOUND;
         outLimb.setParam = 0;
         return true;
      }
      else
      {
         // Basic commands
         for (U32 i=CostumeRenderer::CMD_HIDE; i<CostumeRenderer::CMD_END; i++)
         {
            if (strcasecmp(paramName, CostumeRenderer::opcodeMap[i]) == 0)
            {
               outLimb.setId = 0;
               outLimb.setCommand = i;
               outLimb.setParam = atoi(StringUnit::getUnit(word, 1, ":"));
               return true;
            }
         }
      }
      return false;
   };


   if (!inputStorage->isField)
   {
      if (!outputStorage->isField &&
          (requestedType == TypeLimbControlVector) &&
          inputStorage->data.argc == 1 &&
          inputStorage->data.storageRegister->typeId == TypeLimbControlVector)
      {
         // Just copy data
         U32* ptr = (U32*)ConsoleGetInputStoragePtr();
         U32 numElements = ptr ? *ptr++ : 0;
         U32 dataSize = (numElements * sizeof(CostumeAnim::LimbControl));
         outputStorage->FinalizeStorage(outputStorage, sizeof(U32) +  dataSize);
         U32* returnBuffer = (U32*)ConsoleGetOutputStoragePtr();
         *returnBuffer++ = numElements;
         
         if (ptr)
         {
            memcpy(returnBuffer, ptr, dataSize);
         }
         
         if (outputStorage->data.storageRegister)
         {
            *outputStorage->data.storageRegister = outputStorage->data.storageAddress;
         }
      
         return true;
      }
      else
      {
         // Need to take long path
         vec = &workVec;
         vec->clear();
         
         if (inputStorage->data.argc > 0)
         {
            for (U32 i=0; i<inputStorage->data.argc; i++)
            {
               ConsoleValue val = inputStorage->data.storageRegister[i];

               if (val.typeId == TypeLimbControlVector)
               {
                  U32* storagePtr = (U32*)val.evaluatePtr(vmPtr->getAllocBase());
                  U32 numElems = storagePtr[0];
                  CostumeAnim::LimbControl* data = (CostumeAnim::LimbControl*)(storagePtr+1);
                  
                  for (S32 i=0; i<numElems; i++)
                  {
                     vec->push_back(data[i]);
                  }
               }
               else
               {
                  const char* values = vmPtr->valueAsString(inputStorage->data.storageRegister[i]);
                  if (!values) values = "";

                  U32 numValues = StringUnit::getUnitCount(values, " ");
                  char buffer[128];

                  for (U32 i=0; i<numValues; i++)
                  {
                     CostumeAnim::LimbControl outLimb;
                     const char* word = StringUnit::getUnit(values, i, " ", buffer, 128);
                     if (parseLimb(word, outLimb))
                     {
                        vec->push_back(outLimb);
                     }
                  }
               }
            }
         }
      }
   }
   else
   {
      vec = (std::vector<CostumeAnim::LimbControl>*)ConsoleGetInputStoragePtr();
   }

   // Convert native vector to the requested output.

   if (outputStorage->isField && requestedType == TypeLimbControlVector)
   {
      auto* outputVec = (std::vector<CostumeAnim::LimbControl>*)ConsoleGetOutputStoragePtr();
      *outputVec = *vec;
      return true;
   }
   else if (requestedType == TypeLimbControlVector)
   {
      // Need to convert back to serialized variant
      outputStorage->FinalizeStorage(outputStorage, (vec->size() * sizeof(S32)) + sizeof(CostumeAnim::LimbControl));
      U32* vecCount = (U32*)ConsoleGetOutputStoragePtr();
      *vecCount++ = vec->size();
      
      std::copy(vec->begin(), vec->end(), (CostumeAnim::LimbControl*)vecCount);
      
      if (outputStorage->data.storageRegister)
      {
         *outputStorage->data.storageRegister = outputStorage->data.storageAddress;
      }
      
      return true;
   }
   else if (requestedType != KorkApi::ConsoleValue::TypeInternalString)
   {
      // just treat as empty for now
      outputStorage->data.argc = 1;
      outputStorage->data.storageAddress = KorkApi::ConsoleValue();
      *outputStorage->data.storageRegister = outputStorage->data.storageAddress;
      return true;
   }
   else
   {
      // Serialize to a space-separated string (safe conservative buffer, then finalize).
      S32 maxReturn = 2048;
      outputStorage->ResizeStorage(outputStorage, maxReturn);
      char* returnBuffer = (char*)outputStorage->data.storageAddress.evaluatePtr(vmPtr->getAllocBase());
      returnBuffer[0] = '\0';
      S32 returnLen = 0;
      
      for (U32 i = 0; i < (U32)vec->size(); i++)
      {
         CostumeAnim::LimbControl* control = &(*vec)[i];
         if (control->setCommand >= CostumeRenderer::CMD_END)
            continue;
         
         if (control->setId >= 0)
         {
            snprintf(returnBuffer + returnLen, maxReturn - returnLen,
                     "%s:%i", i == 0 ? "" : " ",
                     CostumeRenderer::opcodeMap[control->setCommand],
                     control->setId);
         }
         else
         {
            snprintf(returnBuffer + returnLen, maxReturn - returnLen,
                     "%s:%u", i == 0 ? "" : " ",
                     CostumeRenderer::opcodeMap[control->setCommand],
                     control->setParam);
         }
         
         returnLen = strlen(returnBuffer);
      }

      outputStorage->FinalizeStorage(outputStorage, returnLen + 1);

      if (outputStorage->data.storageRegister)
         *outputStorage->data.storageRegister = outputStorage->data.storageAddress;

      return true;
   }
   
   return false;
}

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
      mPosition = newPosition;
      mExtent = newExtent;
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
      
      
      for (SimObject* obj : objectList)
      {
         Actor* actor = dynamic_cast<Actor*>(obj);
         if (actor)
         {
            Point2I childPosition = offset + actor->getPosition();
            RectI childClip(childPosition, actor->getExtent());
            actor->onRender(childPosition, childClip, globalCam);
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


void CostumeRenderer::LiveState::render(CostumeRenderer::StaticState& state)
{
   bool doFlip = false;
   
   if ((globalFlags & CostumeRenderer::FLIP) != 0)
  {
     // Ok, we need to flip if the current direction is west
     if (curDirection == CostumeRenderer::WEST)
     {
        doFlip = true;
     }
  }

  for (LimbState& limbState : mLimbState)
  {
     if (limbState.lastEvalFrame < state.mFrames.size() &&
        (limbState.track.flags & HIDE) == 0)
     {
        // Grab frame and draw
        Frame& frame = state.mFrames[limbState.lastEvalFrame];
        Point2F drawPos = position;
        
        TextureSlot* slot = gTextureManager->resolveHandle(frame.displayImage);
        if (slot)
        {
           if (doFlip)
           {
              drawPos += (Point2F(-(frame.displayOffset.x + slot->mTexture.width), frame.displayOffset.y)) * scale;
           }
           else
           {
              drawPos += (Point2F(frame.displayOffset.x, frame.displayOffset.y)) * scale;
           }
           
           ::Rectangle source = {0, 0, (float)slot->mTexture.width, (float)slot->mTexture.height};
           ::Rectangle dest = {drawPos.x, drawPos.y, slot->mTexture.width * scale, slot->mTexture.height * scale};
           Vector2 origin = {};
           
           if (doFlip)
           {
              source.x = (float)slot->mTexture.width;
              source.width = -(float)slot->mTexture.width;
           }
           
           if ((frame.setFlags & SimWorld::ImageSet::FLAG_TRANSPARENT) != 0)
           {
              BeginBlendMode(BLEND_ALPHA);
           }
           
           DrawTexturePro(slot->mTexture, source, dest, origin, 0.0f, WHITE);
           
           if ((frame.setFlags & SimWorld::ImageSet::FLAG_TRANSPARENT) != 0)
           {
              EndBlendMode();
           }
        }
     }
  }
}



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
   
   gFiberManager = new SimFiberManager();
   gFiberManager->registerObject("FiberManager");
   
   Con::addVariable("$VAR_TIMER_NEXT", TypeF32, &gTimerNext);
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

        const float fixedDt = 1.0f / (((float)TICK_HZ) / gTimerNext);
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
                gFiberManager->execFibers(1);
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
            //DrawTexturePro(texture, source, dest, origin, renderState.rotationDeg, WHITE);

            DrawText(TextFormat("Sim tick: %.0f Hz (dt=%.6f) FPS=%i", ((float)TICK_HZ) / gTimerNext, fixedDt, GetFPS()), 10, 10, 20, DARKGRAY);
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
