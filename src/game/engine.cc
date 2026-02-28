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

ConsoleFunctionValue(cursorState, 2, 2, "(value)")
{
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(userPutState, 2, 2, "(value)")
{
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

