//-----------------------------------------------------------------------------
// Copyright (c) 2025-2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//

#include "engine.h"


// --- Tweak these ---
#define TICK_HZ        60.0        // fixed simulation rate (e.g. 60, 64, 128)
#define MAX_FRAME_DT   0.25         // clamp to avoid spiral-of-death (seconds)
#define MAX_STEPS      8            // safety cap: max sim steps per render frame



// globals
F32 gTimerNext = 1.0;
S32 gMouseX = 0.0;
S32 gMouseY = 0.0;
SimFiberManager* gFiberManager = nullptr;
TextureManager* gTextureManager = nullptr;
EngineGlobals gGlobals;

void MyLogger(U32 level, const char *consoleLine, void*)
{
   printf("%s\n", consoleLine);
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
   gGlobals.engineTick.registerTickable();

   gGlobals.sentenceQueue = new SimWorld::SentenceQueueManager();
   gGlobals.sentenceQueue->registerObject("SentenceQueue");
   
   Con::addVariable("$VAR_TIMER_NEXT", TypeF32, &gTimerNext);
   Con::addVariable("$VAR_HAVE_MSG", TypeBool, &gGlobals.currentMessage.ticking);
   Con::addVariable("$VAR_VIRT_MOUSE_X", TypeS32, &gMouseX);
   Con::addVariable("$VAR_VIRT_MOUSE_Y", TypeS32, &gMouseY);
   
   ClearWindowState(FLAG_VSYNC_HINT);
   
   Camera2D cam = {0};
   cam.target = (Vector2){ 0, 0 };   // world origin you want at top-left
   cam.rotation = 0.0f;
   
   const char *fsMaskCutout =
   "#version 330\n"
   "in vec2 fragTexCoord;\n"
   "in vec4 fragColor;\n"
   "out vec4 finalColor;\n"
   "\n"
   "uniform sampler2D texture0;\n"
   "uniform sampler2D maskTex;\n"
   "\n"
   "uniform vec2 rtSizePx;        // e.g. (320, 200)\n"
   "uniform vec2 roomOffsetPx;    // e.g. (0, 28)\n"
   "uniform vec2 roomSizePx;      // e.g. (320, 144)\n"
   "\n"
   "void main()\n"
   "{\n"
   "    vec4 actor = texture(texture0, fragTexCoord) * fragColor;\n"
   "\n"
   "    vec2 p = vec2(gl_FragCoord.x, gl_FragCoord.y);\n"
   "\n"
   "    vec2 uv = p / roomSizePx;\n"
   "    vec4 m = vec4(0.0);\n"
   "    if (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0)\n"
   "    {\n"
   "        m = texture(maskTex, uv);   // 1.0 = white = hide actor\n"
   "    }\n"
   "\n"
   "    actor.a *= 1.0-m.r;\n"
   "    //actor.rgba = vec4(m.r,m.r,m.r,1.0);\n"
   "\n"
   "    finalColor = actor;\n"
   "}\n";
   
   InitWindow(screenWidth, screenHeight, "openquest");
   {
      InitAudioDevice();

      for (U32 i=0; i<AUDIO_CHANNEL_COUNT; i++)
      {
         gGlobals.mChannelVolume[i] = 1.0f;
      }
      
      ClearWindowState(FLAG_VSYNC_HINT);
      
      gGlobals.shaderMask = LoadShaderFromMemory(NULL, fsMaskCutout);
      gGlobals.screenSize = Point2I(screenWidth, screenHeight);
      
      gGlobals.roomRt = LoadRenderTexture(320, 200);
      for (U32 zPlane=0; zPlane<SimWorld::RoomRender::NumZPlanes; zPlane++)
      {
         gGlobals.roomZPlaneRt[zPlane] = LoadRenderTexture(320, 200);
      }
      SetTextureFilter(gGlobals.roomRt.texture, TEXTURE_FILTER_POINT);
      
      Rectangle vp = GetLetterboxViewport(screenWidth, screenHeight, 320, 200);
      float zoom = vp.width / (float)320.0;
      
      cam.offset = (Vector2){ vp.x, vp.y };
      cam.zoom   = zoom;
      
      gTextureManager = new TextureManager();
      
      // Boot
      Con::executef("exec", KorkApi::ConsoleValue::makeString("boot.cs"));
      
      const float fixedDt = 1.0f / (((float)TICK_HZ) / gTimerNext);
      double accumulator = fixedDt;
      
      while (!WindowShouldClose())
      {
         float frameDt = GetFrameTime();
         if (frameDt > (float)MAX_FRAME_DT) frameDt = (float)MAX_FRAME_DT;
         accumulator += frameDt;
         
         if (SimWorld::RootUI::sMainInstance)
         {
            SimWorld::RootUI::sMainInstance->mAnchor = Point2I(0,0);
            SimWorld::RootUI::sMainInstance->mMinContentSize = Point2I(320, 200);
         }
         
         // Update globals
         gMouseX = GetMouseX() - vp.x;
         gMouseY = GetMouseY() - vp.y;
         
         // Need these translated into room space
         gMouseX = ((F32)gMouseX / vp.width) * 320.0;
         gMouseY = ((F32)gMouseY / vp.height) * 200.0;
         
         // Pre-frame layout update
         if (SimWorld::RootUI::sMainInstance)
         {
            SimWorld::RootUI::sMainInstance->updateLayout(RectI(Point2I(0,0), SimWorld::RootUI::sMainInstance->mMinContentSize));
         }
         
         // Call input handler
         if (gGlobals.currentRoom)
         {
            int key = GetKeyPressed();
            while (key > 0)
            {
               Con::executef(gGlobals.currentRoom, "inputHandler", Con::getIntArg(4), Con::getIntArg(0), Con::getIntArg(key));
               key = GetKeyPressed();   // get next key from queue
            }
            
            // NOTE: We will assume room for now
            
            if (IsMouseButtonPressed(0))
            {
               Con::executef(gGlobals.currentRoom, "inputHandler", Con::getIntArg(2), Con::getIntArg(0), Con::getIntArg(1));
            }
            
            if (IsMouseButtonPressed(1))
            {
               Con::executef(gGlobals.currentRoom, "inputHandler", Con::getIntArg(2), Con::getIntArg(0), Con::getIntArg(2));
            }
         }
         
         // Run fixed sim steps as needed
         int steps = 0;
         while (accumulator >= fixedDt && steps < MAX_STEPS)
         {
            ITickable::doFixedTick(fixedDt);
            gFiberManager->execFibers(1);
            accumulator -= fixedDt;
            steps++;
         }

         gGlobals.sentenceQueue->execItem();
         
         BeginDrawing();
         
         ClearBackground(RAYWHITE);
         BeginMode2D(cam);
         
         if (SimWorld::RootUI::sMainInstance)
         {
            SimWorld::RootUI::sMainInstance->onRender(Point2I(0,0), RectI(Point2I(0,0), Point2I(320, 200)), cam);
         }
         
         //DrawText(TextFormat("Sim tick: %.0f Hz (dt=%.6f) FPS=%i", ((float)TICK_HZ) / gTimerNext, fixedDt, GetFPS()), 10, 10, 20, DARKGRAY);
         //DrawText(TextFormat("steps=%d", steps), 10, 35, 20, DARKGRAY);
         
         EndMode2D();
         
         // Debug viewport outline
         DrawRectangleLinesEx(vp, 1, GREEN);
         
         EndDrawing();
      }
   }
   
   Con::shutdown();
   Sim::shutdown();
   
   gGlobals.engineTick.unregisterTickable();
   CloseAudioDevice();
   CloseWindow();
   return 0;
}
