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


struct EngineGlobals
{
   SimWorld::Room*  currentRoom;
   SimWorld::Actor* currentEgo;
   
   RenderTexture2D roomRt;
   Shader shaderMask;
};

extern EngineGlobals gGlobals;

