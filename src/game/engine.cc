#include "engine.h"


void UtilDrawTextLines(const char *text, Point2I pos, int fontSize, int lineSpacing, bool centered, Color color)
{
   static const U32 MaxLines = 10;
   
   struct LineInfo
   {
      S32 offsetX;
      const char* ptr;
   };
   
   if (text == nullptr)
   {
      return;
   }
   
   char buffer[1024];
   strncpy(buffer, text, sizeof(buffer)-1);
   buffer[sizeof(buffer)-1] = '\0';
   
   char *line = buffer;
   S32 offsetY = 0;
   
   U32 numLines = 1;
   LineInfo lineInfo[MaxLines];
   
   lineInfo[0].ptr = buffer;
   lineInfo[0].offsetX = 0;
   
   for (char *p = buffer; ; p++)
   {
      if (*p == '\n' || *p == '\0')
      {
         char saved = *p;
         *p = '\0';
         
         if (centered)
         {
            int textLength = ::MeasureText(lineInfo[numLines-1].ptr,
                                           fontSize);
            lineInfo[numLines-1].offsetX = -textLength / 2;
         }
         
         if (numLines >= MaxLines)
            break;
         
         if (saved == '\0')
            break;         // finished
         
         lineInfo[numLines].ptr = p+1;
         lineInfo[numLines].offsetX = 0;
         numLines++;
      }
   }
   
   // handle trailing line
   if (centered)
   {
      int textLength = ::MeasureText(lineInfo[numLines-1].ptr,
                                     fontSize);
      lineInfo[numLines-1].offsetX = -textLength / 2;
   }
   
   // Make sure we have enough room for lines
   offsetY -= (fontSize + lineSpacing) * numLines;
   
   // Draw all lines from top to bottom
   for (U32 i=0; i<numLines; i++)
   {
      LineInfo& line = lineInfo[i];
      ::DrawText(line.ptr, pos.x + line.offsetX, pos.y + offsetY, fontSize, color);
      offsetY += fontSize + lineSpacing;
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
   tick = tickLength;
   actor = nullptr;
   sound = nullptr;

   if (saveActor)
   {
      saveActor->stopTalk();
   }
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
