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
SimFiberManager* gFiberManager = nullptr;
TextureManager* gTextureManager = nullptr;

static Input SampleInput(void)
{
    Input in = { 0 };

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  in.moveX -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) in.moveX += 1;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    in.moveY -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  in.moveY += 1;

    return in;
}

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
                ITickable::doFixedTick(fixedDt);
                gFiberManager->execFibers(1);
                accumulator -= fixedDt;
                steps++;
            }

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

            //Rectangle dest = { renderState.pos.x, renderState.pos.y, size.x, size.y };
            //DrawTexturePro(texture, source, dest, origin, renderState.rotationDeg, WHITE);

            DrawText(TextFormat("Sim tick: %.0f Hz (dt=%.6f) FPS=%i", ((float)TICK_HZ) / gTimerNext, fixedDt, GetFPS()), 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("steps=%d", steps), 10, 35, 20, DARKGRAY);
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
