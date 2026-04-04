#pragma once

#include "raylib.h"
#include "raygui.h"

#include <string.h>
#include <stdio.h>

#include "game/displayBase.h"

class ConsoleWidget : public SimWorld::DisplayBase
{
typedef DisplayBase Parent;

public:

	enum
	{
		MAX_LOG_LINES=512,
		MAX_LINE_LEN=256,
		INPUT_LEN=256,
		HISTORY_SIZE=128
	};

    // log
    char mLogLines[MAX_LOG_LINES][MAX_LINE_LEN];
    int mLogCount;

    // input
    char mInput[INPUT_LEN];
    bool mInputEditMode;

    // history
    char mHistory[HISTORY_SIZE][INPUT_LEN];
    int mHistoryCount;
    int mHistoryIndex;          // -1 means not browsing history
    char mHistoryScratch[INPUT_LEN];

    // scrolling
    Vector2 mScroll;
    bool mAutoScroll;

    // layout cache
    Rectangle mLastView;
    float mLineHeight;
    float mInputHeight;
    float mLinePadding;

    ConsoleWidget();
    ~ConsoleWidget();

    void clear();
    void pushHistory(const char *command);
    void executeInput();
    void handleHistory();

    void onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera) override;


   bool onAdd() override;
   void onRemove() override;

   static void initPersistFields();
   static void ConsumeLog(U32 level, const char* consoleLine, void* userPtr);

   DECLARE_CONOBJECT(ConsoleWidget);
};
