#include "game/consoleWidget.h"

IMPLEMENT_CONOBJECT(ConsoleWidget);

ConsoleWidget::ConsoleWidget()
{
   mHistoryIndex = -1;
   mHistoryCount = 0;
   mAutoScroll = true;
   mLineHeight = 18.0f;
   mInputHeight = 26.0f;
   mLinePadding = 4.0f;
   
   clear();
}

ConsoleWidget::~ConsoleWidget()
{
   
}

void ConsoleWidget::initPersistFields()
{
   Parent::initDisplayFields();
   initDisplayFields();
}

bool ConsoleWidget::onAdd()
{
   if (Parent::onAdd())
   {
      Con::addConsumer(ConsumeLog, this);
      gGlobals.consoleInput = true;
      return true;
   }
   return false;
}

void ConsoleWidget::onRemove()
{
   Con::removeConsumer(ConsumeLog, this);
   Parent::onRemove();
}

void ConsoleWidget::ConsumeLog(U32 level, const char* consoleLine, void* userPtr)
{
   if (!consoleLine)
   {
      return;
   }
   
   ConsoleWidget* widget = (ConsoleWidget*)userPtr;
   
   if (widget->mLogCount < MAX_LOG_LINES)
   {
      strncpy(widget->mLogLines[widget->mLogCount], consoleLine, ConsoleWidget::MAX_LINE_LEN - 1);
      widget->mLogLines[widget->mLogCount][ConsoleWidget::MAX_LINE_LEN - 1] = '\0';
      widget->mLogCount++;
   }
   else
   {
      for (int i = 1; i < ConsoleWidget::MAX_LOG_LINES; i++)
      {
         memcpy(widget->mLogLines[i - 1], widget->mLogLines[i], ConsoleWidget::MAX_LINE_LEN);
      }
      
      strncpy(widget->mLogLines[ConsoleWidget::MAX_LOG_LINES - 1], consoleLine, ConsoleWidget::MAX_LINE_LEN - 1);
      widget->mLogLines[ConsoleWidget::MAX_LOG_LINES - 1][ConsoleWidget::MAX_LINE_LEN - 1] = '\0';
   }
   
   widget->mAutoScroll = true;
}

void ConsoleWidget::clear()
{
   mLogCount = 0;
   mScroll = {};
   mAutoScroll = true;
   mHistoryCount = 0;
   mHistoryIndex = -1;
   
   memset(mLogLines, '\0', sizeof(ConsoleWidget::mLogLines));
   memset(mHistory, '\0', sizeof(ConsoleWidget::mHistory));
   memset(mInput, '\0', sizeof(ConsoleWidget::mInput));
}

void ConsoleWidget::pushHistory(const char *command)
{
   if (!command || command[0] == '\0') return;
   
   // Avoid pushing duplicate consecutive commands
   if (mHistoryCount > 0 &&
       strcmp(mHistory[mHistoryCount - 1], command) == 0)
   {
      mHistoryIndex = -1;
      return;
   }
   
   if (mHistoryCount < ConsoleWidget::HISTORY_SIZE)
   {
      strncpy(mHistory[mHistoryCount], command, ConsoleWidget::INPUT_LEN - 1);
      mHistory[mHistoryCount][ConsoleWidget::INPUT_LEN - 1] = '\0';
      mHistoryCount++;
   }
   else
   {
      for (int i = 1; i < ConsoleWidget::HISTORY_SIZE; i++)
      {
         memcpy(mHistory[i - 1], mHistory[i], ConsoleWidget::INPUT_LEN);
      }
      
      strncpy(mHistory[ConsoleWidget::HISTORY_SIZE - 1], command, ConsoleWidget::INPUT_LEN - 1);
      mHistory[ConsoleWidget::HISTORY_SIZE - 1][ConsoleWidget::INPUT_LEN - 1] = '\0';
   }
   
   mHistoryIndex = -1;
}

void ConsoleWidget::executeInput()
{
   if (mInput[0] == '\0') return;
   
   char realInput[1024];
   snprintf(realInput, sizeof(realInput), "> %s", mInput);
   ConsumeLog(0, realInput, this);
   pushHistory(mInput);
   
   Con::evaluatef(mInput);
   
   mInput[0] = '\0';
   mHistoryScratch[0] = '\0';
   mHistoryIndex = -1;
}

void ConsoleWidget::handleHistory()
{
   if (!mInputEditMode) return;
   if (mHistoryCount <= 0) return;
   
   if (IsKeyPressed(KEY_UP))
   {
      if (mHistoryIndex == -1)
      {
         strncpy(mHistoryScratch, mInput, ConsoleWidget::INPUT_LEN - 1);
         mHistoryScratch[ConsoleWidget::INPUT_LEN - 1] = '\0';
         mHistoryIndex = mHistoryCount - 1;
      }
      else if (mHistoryIndex > 0)
      {
         mHistoryIndex--;
      }
      
      strncpy(mInput, mHistory[mHistoryIndex], ConsoleWidget::INPUT_LEN - 1);
      mInput[ConsoleWidget::INPUT_LEN - 1] = '\0';
   }
   
   if (IsKeyPressed(KEY_DOWN))
   {
      if (mHistoryIndex != -1)
      {
         if (mHistoryIndex < mHistoryCount - 1)
         {
            mHistoryIndex++;
            strncpy(mInput, mHistory[mHistoryIndex], ConsoleWidget::INPUT_LEN - 1);
            mInput[ConsoleWidget::INPUT_LEN - 1] = '\0';
         }
         else
         {
            mHistoryIndex = -1;
            strncpy(mInput, mHistoryScratch, ConsoleWidget::INPUT_LEN - 1);
            mInput[ConsoleWidget::INPUT_LEN - 1] = '\0';
         }
      }
   }
}

void ConsoleWidget::onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera)
{
   bool active = true;
   mInputEditMode = active;
   
   const float pad = mLinePadding;
   const float inputH = mInputHeight;
   
   Camera2D copyCamera = globalCamera;
   copyCamera.zoom = 1.0;
   RectI trueBounds = WorldRectToScreen(mBounds, globalCamera);
   trueBounds.point = Point2I(0,0);
   
   EndMode2D();
   BeginMode2D(copyCamera);
   
   Rectangle logBounds = {
      (float)trueBounds.point.x,
      (float)trueBounds.point.y,
      (float)trueBounds.extent.x,
      (float)(trueBounds.extent.y - inputH - pad)
   };
   
   Rectangle inputBounds = {
      (float)trueBounds.point.x,
      (float)(trueBounds.point.y + trueBounds.extent.y - inputH),
      (float)trueBounds.extent.x,
      inputH
   };
   
   Rectangle contentRect = {
      0,
      0,
      logBounds.width - 16, // leave room for scrollbar
      (float)mLogCount * mLineHeight + pad * 2
   };
   
   Rectangle view = { 0 };
   
   // Make scroll panel background transparent by disabling panel fill/border colors temporarily.
   int prevBaseNormal = GuiGetStyle(DEFAULT, BACKGROUND_COLOR);
   int prevBorderNormal = GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL);
   int prevBorderFocused = GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED);
   int prevBorderPressed = GuiGetStyle(DEFAULT, BORDER_COLOR_PRESSED);
   
   GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt((Color){ 0, 0, 0, 0 }));
   GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt((Color){ 0, 0, 0, 0 }));
   GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, ColorToInt((Color){ 0, 0, 0, 0 }));
   GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, ColorToInt((Color){ 0, 0, 0, 0 }));
   
   DrawRectangleGradientV(
       (int)logBounds.x, (int)logBounds.y,
       (int)logBounds.width, (int)logBounds.height,
       (Color){ 20, 20, 30, 180 },
       (Color){ 10, 10, 15, 140 }
   );
   
   GuiScrollPanel(logBounds, NULL, contentRect, &mScroll, &view);
   
   GuiSetStyle(DEFAULT, BACKGROUND_COLOR, prevBaseNormal);
   GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, prevBorderNormal);
   GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, prevBorderFocused);
   GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, prevBorderPressed);
   
   mLastView = view;
   
   // Mouse wheel scroll when hovering log area
   if (CheckCollisionPointRec(GetMousePosition(), logBounds))
   {
      float wheel = GetMouseWheelMove();
      if (wheel != 0.0f)
      {
         mScroll.y -= wheel * mLineHeight * 3.0f;
         mAutoScroll = false;
      }
   }
   
   // Clamp scroll
   float maxScrollY = contentRect.height - view.height;
   if (maxScrollY < 0) maxScrollY = 0;

   // raygui scroll range is [-maxScrollY, 0]
   if (mScroll.y > 0) mScroll.y = 0;
   if (mScroll.y < -maxScrollY) mScroll.y = -maxScrollY;
   
   // Auto-scroll to bottom when new lines arrive
   if (mAutoScroll)
   {
      mScroll.y = -maxScrollY;
      mAutoScroll = false;
   }
   
   // Draw text inside clipped view
   
   BeginScissorMode(view.x, view.y, view.width, view.height);
   {
      int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
      int spacing = GuiGetStyle(DEFAULT, TEXT_SPACING);
      
      for (int i = 0; i < mLogCount; i++)
      {
         float y = logBounds.y + pad + i * mLineHeight + mScroll.y;
         if (y + mLineHeight < view.y) continue;
         if (y > view.y + view.height) break;
         
         DrawTextEx(
                    GetFontDefault(),
                    mLogLines[i],
                    (Vector2){ logBounds.x + pad, y },
                    (float)fontSize,
                    (float)spacing,
                    (Color){255,255,255,255}
                    );
      }
   }
   EndScissorMode();
   
   // Input box
   bool submittedByGui = GuiTextBox(inputBounds, mInput, ConsoleWidget::INPUT_LEN, mInputEditMode);
   
   // Optional: click input to focus
   if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
   {
      mInputEditMode = CheckCollisionPointRec(GetMousePosition(), inputBounds) ? true : active;
   }
   
   handleHistory();
   
   // Enter executes
   if (mInputEditMode && IsKeyPressed(KEY_ENTER))
   {
      executeInput();
   }
   
   // Fallback
   if (submittedByGui && mInputEditMode && IsKeyPressed(KEY_ENTER))
   {
      executeInput();
   }
   
   // Reset mode
   EndMode2D();
   BeginMode2D(globalCamera);
}
