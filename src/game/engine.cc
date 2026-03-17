#include "engine.h"
#include "math/mPointTypeTraits.h"

template <>
struct PointTraits<Color>
{
   static constexpr int  N      = 4;
   
   static S32* typeIdPtr() { return &TypeColor; }

   static void zero(Color& p) { p = (Color){0,0,0,0}; }

   static U8& at(Color& p, int i)
   {
      return ((U8*)&p)[i];
   }

   static const char* scanFmt()  { return "%hhu %hhu %hhu %hhu"; }
   static const char* printFmt() { return "%hhu %hhu %hhu %hhu"; }
};


ConsoleType( Color, TypeColor, sizeof(Color), sizeof(Color), "" )
ConsoleTypeOpDefault(TypeColor)

ConsoleGetType( TypeColor )
{
   return getPointDataImpl<Color>(vmPtr, inputStorage, outputStorage, fieldUserPtr, flag, requestedType);
}

void UtilDrawOutlinedText(const char *text, S32 posX, S32 posY, S32 fontSize, Color color, S32 outlineSize, Color outlineColor)
{
   if (outlineSize > 0)
   {
      Vector2 offsets[8] = {
          {-1,-1},{0,-1},{1,-1},
          {-1, 0},        {1, 0},
          {-1, 1},{0, 1},{1, 1}
      };

      for (int i = 0; i < 8; i++)
      {
          DrawText(text,
              posX + offsets[i].x * outlineSize,
              posY + offsets[i].y * outlineSize,
              fontSize,
              outlineColor);
      }
   }
   
   DrawText(text, posX, posY, fontSize, color);
}

void UtilDrawTextLines(const char *text,
                       Point2I pos,
                       int fontSize,
                       int lineSpacing,
                       bool centered,
                       Color color,
                       Point2I screenSize,
                       int margin,
                       int maxLines,
                       int maxTextWidth)
{
   struct LineInfo
   {
      const char *ptr;
      S32 offsetX;
   };

   if (text == nullptr || *text == '\0' || maxLines <= 0)
      return;

   // Fallback width: keep text inside margins.
   if (maxTextWidth <= 0)
      maxTextWidth = screenSize.x - margin * 2;

   if (maxTextWidth < 1)
      return;

   // Working buffer
   char buffer[1024];
   strncpy(buffer, text, sizeof(buffer) - 1);
   buffer[sizeof(buffer) - 1] = '\0';

   LineInfo lineInfo[64]; // hard upper bound for safety
   const S32 lineCapacity = std::min(maxLines, (int)(sizeof(lineInfo) / sizeof(lineInfo[0])));

   S32 numLines = 0;
   char *p = buffer;

   auto pushLine = [&](char *lineStart)
   {
      if (numLines >= lineCapacity || lineStart == nullptr)
      {
         return;
      }

      // Trim leading spaces on wrapped lines.
      while (*lineStart == ' ')
      {
         ++lineStart;
      }

      lineInfo[numLines].ptr = lineStart;
      lineInfo[numLines].offsetX = 0;

      if (centered)
      {
         S32 textLength = ::MeasureText(lineStart, fontSize);
         lineInfo[numLines].offsetX = -textLength / 2;
      }

      ++numLines;
   };

   while (*p && numLines < lineCapacity)
   {
      char *lineStart = p;
      char *lastBreakable = nullptr;
      char *lastNonSpace = nullptr;
      bool nextLine = false;

      // Respect explicit newlines first.
      while (*p && *p != '\n')
      {
         if (*p == ' ' || *p == '\t')
         {
            lastBreakable = p;
         }
         else
         {
            lastNonSpace = p;
         }

         S32 currentWidth = ::MeasureText(lineStart, fontSize);

         // Too wide: wrap.
         if (currentWidth > maxTextWidth)
         {
            if (lastBreakable && lastBreakable >= lineStart)
            {
               // Wrap at last space/tab.
               char *nextStart = lastBreakable + 1;
               *lastBreakable = '\0';
               pushLine(lineStart);
               p = nextStart;
            }
            else
            {
               // No breakable space: hard-wrap inside a long word.
               // Step back until it fits, leaving at least one char.
               char *split = p;
               while (split > lineStart + 1)
               {
                  char saved = *split;
                  *split = '\0';
                  S32 width = ::MeasureText(lineStart, fontSize);
                  *split = saved;

                  if (width <= maxTextWidth)
                     break;

                  --split;
               }

               if (split <= lineStart)
               {
                  split = lineStart + 1;
               }

               char saved = *split;
               *split = '\0';
               pushLine(lineStart);
               *split = saved;
               p = split;
            }

            nextLine = true;
            break;
         }

         ++p;
      }

      if (!nextLine)
      {

         // End of explicit line or end of string.
         if (*p == '\n')
         {
            *p = '\0';

            // Trim trailing spaces.
            if (lastNonSpace)
            {
               *(lastNonSpace + 1) = '\0';
            }

            pushLine(lineStart);
            ++p;
         }
         else
         {
            // End of string.
            if (lastNonSpace)
            {
               *(lastNonSpace + 1) = '\0';
            }

            pushLine(lineStart);
         }
      }
   }

   if (numLines == 0)
      return;

   // Height of the full text block.
   const S32 lineAdvance = fontSize + lineSpacing;
   const S32 textBlockHeight = numLines * lineAdvance;
   
   S32 startY = pos.y - textBlockHeight;

   // Clamp vertically so the whole block stays on-screen.
   if (startY < margin)
   {
      startY = margin;
   }

   S32 bottomY = startY + textBlockHeight;
   if (bottomY > screenSize.y - margin)
   {
      startY -= (bottomY - (screenSize.y - margin));
   }

   if (startY < margin)
   {
      startY = margin;
   }

   // Draw all lines.
   S32 drawY = startY;
   for (S32 i = 0; i < numLines; ++i)
   {
      const LineInfo &line = lineInfo[i];
      S32 lineWidth = ::MeasureText(line.ptr, fontSize);

      S32 drawX;
      if (centered)
      {
         drawX = pos.x + line.offsetX;

         // Clamp centered line so it remains fully visible.
         if (drawX < margin)
         {
            drawX = margin;
         }
         if (drawX + lineWidth > screenSize.x - margin)
         {
            drawX = screenSize.x - margin - lineWidth;
         }
      }
      else
      {
         drawX = pos.x;

         // Clamp left-aligned line.
         if (drawX < margin)
         {
            drawX = margin;
         }
         if (drawX + lineWidth > screenSize.x - margin)
         {
            drawX = screenSize.x - margin - lineWidth;
         }
      }

      // If a single line is still wider than maxTextWidth due to font oddities,
      // keep it anchored at the margin rather than letting it go off-screen.
      if (drawX < margin)
      {
         drawX = margin;
      }

      UtilDrawOutlinedText(line.ptr, drawX, drawY, fontSize, color, 1, BLACK);
      drawY += lineAdvance;
   }
}

bool ActiveMessage::isCompleted()
{
   return tick > tickLength;
}

void ActiveMessage::onStop()
{
   SimWorld::Actor* saveActor = actor;
   
   ticking = false;
   tick = tickLength+1;
   actor = nullptr;
   sound = nullptr;

   if (saveActor)
   {
      saveActor->stopTalk();
   }


   gFiberManager->mFiberGlobalFlags &= ~SCHEDULE_FLAG_MESSAGE;
}


void ActiveMessage::onStart(MessageDisplayParams& newParams, SimWorld::Actor* newActor, SimWorld::Sound* newSound, StringTableEntry newMessage, bool isTalk, U32 ovrTicks)
{
   params = newParams;
   actor = newActor;
   sound = newSound;
   message = newMessage;
   tick = 0;
   ticking = true;
   talking = isTalk;

   gFiberManager->mFiberGlobalFlags |= SCHEDULE_FLAG_MESSAGE;
   
   if (ovrTicks == 0)
   {
      tickLength = newParams.tickSpeed;
      tickLength *= strlen(newMessage);
   }
   else
   {
      tickLength = ovrTicks;
   }

   if (newActor)
   {
      newActor->startTalk();
   }
}

void EngineTickable::onFixedTick(F32 dt)
{
   if (gGlobals.currentMessage.ticking  &&
       !gGlobals.currentMessage.isCompleted())
   {
      gGlobals.currentMessage.tick++;
      gGlobals.currentMessage.ticking = !gGlobals.currentMessage.isCompleted();
      if (!gGlobals.currentMessage.ticking)
      {
         gGlobals.currentMessage.onStop();
      }
   }
}

void EngineGlobals::setActiveMessage(MessageDisplayParams params, SimWorld::Actor* actor, SimWorld::Sound* sound, StringTableEntry message, bool isTalk, U32 ovrTicks)
{
   // Stop message
   if (!currentMessage.isCompleted())
   {
      currentMessage.onStop();
   }
   
   currentMessage.onStart(params, actor, sound, message, isTalk, ovrTicks);
}

ConsoleFunctionValue(setFiberSuspendMode, 2, 2, "(mode)")
{
   gFiberManager->setSuspendMode(vmPtr->valueAsInt(argv[1]));
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(throwFibersWithMask, 3, 4, "(mask, catchMask, soft)")
{
   U32 catchValue = vmPtr->valueAsInt(argv[2]);
   if (argc > 3 && vmPtr->valueAsBool(argv[3]))
   {
      catchValue |= BIT(31);
   }
   gFiberManager->throwWithMask(vmPtr->valueAsInt(argv[1]), catchValue);
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(throwFibersWithObject, 3, 4, "(object, catchMask, soft)")
{
   U32 catchValue = vmPtr->valueAsInt(argv[2]);
   if (argc > 3 && vmPtr->valueAsBool(argv[3]))
   {
      catchValue |= BIT(31);
   }
   gFiberManager->throwWithObject(vmPtr->valueAsInt(argv[1]), catchValue);
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(startCutscene, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(cursorState, 2, 2, "(value)")
{
   gGlobals.cursorState = vmPtr->valueAsBool(argv[1]);
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(userPutState, 2, 2, "(value)")
{
   gGlobals.userPut = vmPtr->valueAsBool(argv[1]);
   return KorkApi::ConsoleValue();
}


RaylibInputRouter::RaylibInputRouter(SimWorld::DisplayBase* root)
: mRoot(root)
{
   const Vector2 mouseR = GetMousePosition();
   mLastMouse = Point2I(mouseR.x, mouseR.y);
}

RaylibInputRouter::~RaylibInputRouter()
{
   
}

void RaylibInputRouter::update(Camera2D& cam)
{
   if (!mRoot) return;
   
   const Vector2 mouseR = GetMousePosition();
   Point2I mouse(mouseR.x, mouseR.y);
   
   mouse = ScreenPointToWorld(mouse, cam);
   
   SimWorld::DisplayBase* capturedControl = mLastEvent.capturedControl;
   
   mLastEvent = {};
   mLastEvent.capturedControl = capturedControl;
   
   if (mouse.x != mLastMouse.x || mouse.y != mLastMouse.y)
   {
      mLastEvent.type = UI_EVENT_MOUSE_MOVE;
      mLastEvent.handled = false;
      mLastEvent.mouse.pos = mouse;
      mLastEvent.mouse.button = -1;
      mLastEvent.mouse.wheelPos = 0.0f;
      
      mRoot->processInput(mLastEvent);
      
      mLastMouse = mouse;
   }
   
   const float wheel = GetMouseWheelMove();
   if (wheel != 0.0f)
   {
      DBIEvent e{};
      mLastEvent.type = UI_EVENT_MOUSE_WHEEL;
      mLastEvent.handled = false;
      mLastEvent.mouse.pos = mouse;
      mLastEvent.mouse.button = -1;
      mLastEvent.mouse.wheelPos = wheel;
      
      mRoot->processInput(mLastEvent);
   }
   
   for (int b = 0; b <= MOUSE_BUTTON_MIDDLE; ++b)
   {
      if (IsMouseButtonPressed(b))
      {
         mActiveMouseButtons.insert(b);
         
         DBIEvent e{};
         mLastEvent.type = UI_EVENT_MOUSE_DOWN;
         mLastEvent.handled = false;
         mLastEvent.mouse.pos = mouse;
         mLastEvent.mouse.button = b;
         mLastEvent.mouse.wheelPos = 0.0f;
         
         mRoot->processInput(mLastEvent);
      }
   }
   
   if (!mActiveMouseButtons.empty())
   {
      for (auto it = mActiveMouseButtons.begin(); it != mActiveMouseButtons.end(); )
      {
         const int b = *it;
         if (IsMouseButtonReleased(b))
         {
            mLastEvent.type = UI_EVENT_MOUSE_UP;
            mLastEvent.handled = false;
            mLastEvent.mouse.pos = mouse;
            mLastEvent.mouse.button = b;
            mLastEvent.mouse.wheelPos = 0.0f;
            
            mRoot->processInput(mLastEvent);
            
            it = mActiveMouseButtons.erase(it);
         }
         else
         {
            ++it;
         }
      }
   }
   
   for (int k = GetKeyPressed(); k != 0; k = GetKeyPressed())
   {
      mActiveKeys.insert(k);
      
      mLastEvent.type = UI_EVENT_KEY_DOWN;
      mLastEvent.handled = false;
      mLastEvent.keys.key = (U32)k;
      
      mRoot->processInput(mLastEvent);
   }
   
   if (!mActiveKeys.empty())
   {
      for (auto it = mActiveKeys.begin(); it != mActiveKeys.end(); )
      {
         const int k = *it;
         if (IsKeyReleased(k))
         {
            mLastEvent.type = UI_EVENT_KEY_UP;
            mLastEvent.handled = false;
            mLastEvent.keys.key = (U32)k;
            
            mRoot->processInput(mLastEvent);
            
            it = mActiveKeys.erase(it);
         }
         else
         {
            ++it;
         }
      }
   }
   
   for (int cp = GetCharPressed(); cp != 0; cp = GetCharPressed())
   {
      mLastEvent.type = UI_EVENT_CHAR;
      mLastEvent.handled = false;
      mLastEvent.keys.codePoint = (U32)cp;
      
      mRoot->processInput(mLastEvent);
   }
}

