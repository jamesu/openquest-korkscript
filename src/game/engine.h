#pragma once

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


#define BEGIN_SW_NS namespace SimWorld {
#define END_SW_NS }

// NOTE: rather than playing the "did we include the right header" game, this includes everything

#include "platform/platform.h"

#include <math.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <algorithm>

#include "platform/platformProcess.h"

#include "math/mPoint.h"
#include "math/mRect.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "math/mathTypes.h"
#include "sim/simBase.h"
#include "core/freeListHandleHelpers.h"
#include "core/memStream.h"
#include "core/fileStream.h"
#include "core/stringUnit.h"
#include "sim/simFiberManager.h"


// raylib stuff

#include "raylib.h"
#include "raygui.h"


// misc shared defs


// Flags for waiting fibers
#define SCHEDULE_FLAG_MESSAGE       BIT(8)
#define SCHEDULE_FLAG_CAMERA_MOVING BIT(9)
#define SCHEDULE_FLAG_SENTENCE_BUSY BIT(10)

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


struct ColorI
{
   U8 r;
   U8 g;
   U8 b;
   U8 a;
};

enum Direction : U8
{
   NORTH,
   SOUTH,
   WEST,
   EAST
};

enum AudioChannels : U8
{
    AUDIO_CHANNEL_SFX,
    AUDIO_CHANNEL_VOICE,
    AUDIO_CHANNEL_MUSIC,
    AUDIO_CHANNEL_COUNT
};


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

struct MessageDisplayParams
{
   Color displayColor;
   Point2I messageOffset;
   U32 fontSize;
   U32 lineSpacing;
   U32 tickSpeed;
   bool relative;
   bool centered;
};

static inline Point2I WorldPointToScreen(Point2I point, Camera2D cam)
{
    Vector2 topLeft     = GetWorldToScreen2D((Vector2){ (float)(point.x), (float)(point.y) }, cam);

    return Point2I(
        topLeft.x,
        topLeft.y
    );
}

static inline RectI WorldRectToScreen(RectI r, Camera2D cam)
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

static inline Rectangle RectIToRectangle(RectI r)
{
   return (Rectangle){(float)r.point.x, (float)r.point.y, (float)r.extent.x, (float)r.extent.y};
}

static inline Camera2D MakeDefaultCamera()
{
   Camera2D cam = {};
   cam.zoom = 1.0;
   return cam;
}

void UtilDrawTextLines(const char *text, Point2I pos, int fontSize, int lineSpacing, bool centered, Color color);

// engine objects and apis...

BEGIN_SW_NS

class Actor;
struct ActorWalkState;
class Costume;
struct CostumeRenderer;
class Room;
class Verb;
class ImageSet;
class Sound;

END_SW_NS

#include "scheduling.h"
#include "displayBase.h"
#include "resources.h"
#include "resourceManagers.h"
#include "costume.h"
#include "actor.h"
#include "room.h"
#include "sound.h"
#include "verbs.h"


// globals

extern F32 gTimerNext;

extern SimFiberManager* gFiberManager;
extern TextureManager* gTextureManager;

struct SentenceQueueItem
{
    SimObjectId verb;
    SimObjectId objA;
    SimObjectId objB;
};

struct ActiveMessage
{
    MessageDisplayParams params;
    SimWorld::Actor* actor;
    SimWorld::Sound* sound;
    StringTableEntry message;
    U32 tickLength;
    U32 tick;
    bool ticking;
    bool talking;

    bool isCompleted();
    void onStop();
    void onStart(MessageDisplayParams& newParams, SimWorld::Actor* newActor, SimWorld::Sound* newSound, StringTableEntry newMessage, bool isTalk, U32 ovrTicks=0);
};

// This handles engine ticks
class EngineTickable : public ITickable
{
public:
   
   void onFixedTick(F32 dt) override;
};

// Handy set of globals
struct EngineGlobals
{
   SimWorld::Room*  currentRoom;
   SimWorld::Actor* currentEgo;
   EngineTickable engineTick;

   KorkApi::FiberId sentenceFiber;
   std::vector<SentenceQueueItem> sentenceQueue;
   ActiveMessage currentMessage;
   
   RenderTexture2D roomRt;
   RenderTexture2D roomZPlaneRt[SimWorld::RoomRender::NumZPlanes];
   Shader shaderMask;

   F32 mChannelVolume[AUDIO_CHANNEL_COUNT];

   U32 messageSpeed;
   
   Point2I screenSize;

   void setActiveMessage(MessageDisplayParams params, SimWorld::Actor* actor, SimWorld::Sound* sound, StringTableEntry message, bool isTalk, U32 ovrTicks);
};



extern EngineGlobals gGlobals;

