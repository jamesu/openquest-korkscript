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

TextureManager* gTextureManager = NULL;

TextureSlot* TextureHandle::getPtr() const
{
    return gTextureManager->resolveHandle(*this);
}

struct RoomRender
{

   enum
   {
       NumZPlanes = 3
   };
    TextureHandle backgroundImage;
    TextureHandle zPlanes[3];

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
   
   void enumerateItems(std::vector<CostumeAnim*> &anims);
public:
   DECLARE_CONOBJECT(Costume);
};

IMPLEMENT_CONOBJECT(Costume);

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
   Direction mDirection;
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
   
   Costume* mCostume;
   CostumeRenderer::LiveState mLiveCostume;
   
   
   DECLARE_CONOBJECT(Actor);
};

IMPLEMENT_CONOBJECT(Actor);

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
   
   void renderChildren(Point2I offset, RectI drawRect)
   {
      for (SimObject* obj : objectList)
      {
         DisplayBase* dObj = dynamic_cast<DisplayBase*>(obj);
         if (dObj)
         {
            Point2I childPosition = offset + dObj->getPosition();
            RectI childClip(childPosition, dObj->getExtent());
            
            dObj->onRender(offset, drawRect);
         }
      }
   }
   
   virtual void onRender(Point2I offset, RectI drawRect)
   {
   }
   
   
public:
   DECLARE_CONOBJECT(DisplayBase);
};

IMPLEMENT_CONOBJECT(DisplayBase);

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
   DECLARE_CONOBJECT(RootUI);
};

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


class Room : public DisplayBase
{
   typedef SimObject Parent;
public:

    StringTableEntry mImageFileName;
    StringTableEntry mBoxFileName;
    StringTableEntry mZPlaneFiles[RoomRender::NumZPlanes];

    U32 mTransFlags;
   
   RoomRender mRenderState;
   
   void updateResources()
   {
      mRenderState.backgroundImage = mImageFileName && *mImageFileName ? gTextureManager->loadTexture(mImageFileName) : nullptr;
      for (uint32_t i=0; i<RoomRender::NumZPlanes; i++)
      {
         mRenderState.zPlanes[i] = mZPlaneFiles[i] && *mZPlaneFiles[i] ? gTextureManager->loadTexture(mZPlaneFiles[i]) : nullptr;
      }
   }
   
   virtual void onRender(Point2I offset, RectI drawRect)
   {
      if (mRenderState.backgroundImage.getIndex() == 0)
      {
         updateResources();
      }
      
      
   }

public:
   DECLARE_CONOBJECT(Room);
};
IMPLEMENT_CONOBJECT(Room);

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

int main(int argc, char **argv)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    Con::init();
    Sim::init();
    Con::addConsumer(MyLogger, NULL);
   ClearWindowState(FLAG_VSYNC_HINT);

    InitWindow(screenWidth, screenHeight, "raylib - fixed tick sim + variable render");
    {
       ClearWindowState(FLAG_VSYNC_HINT);
         SetTargetFPS(1000); // render as fast as possible (optional)

        gTextureManager = new TextureManager();

        // Boot
        Con::executef(2, "exec", "boot.cs");

        TextureHandle rectTex = gTextureManager->loadTexture("graphics/original_art/back01.bmp");
        const Texture2D& texture = rectTex.getPtr()->mTexture;

        // Rectangle draw setup
        Vector2 size = { (float)texture.width, (float)texture.height };
        Vector2 origin = { size.x / 2.0f, size.y / 2.0f };

        const float fixedDt = 1.0f / (float)TICK_HZ;
        double accumulator = 0.0;

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

            // Run fixed sim steps as needed
            int steps = 0;
            while (accumulator >= fixedDt && steps < MAX_STEPS)
            {
                prev = curr;                 // keep last state for interpolation
                UpdateFixed(&curr, in, fixedDt);
                accumulator -= fixedDt;
                steps++;
            }

            // Interpolation factor for smooth rendering
            float alpha = (float)(accumulator / fixedDt);
            State renderState = LerpState(prev, curr, alpha);

            BeginDrawing();
            ClearBackground(RAYWHITE);
           
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

            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}
