#pragma once

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS

class VerbDisplay : public DisplayBase
{
   typedef DisplayBase Parent;
public:
   
   StringTableEntry mDisplayText;
   StringTableEntry mVerbName; // verb to set
   
   RoomObject* mRoomObject; // room object to draw
   
   VerbDisplay();
   
   bool onAdd();
   void onRemove();
   
   bool processInput(DBIEvent& event);
   void onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera);
   
   void updateLayout(const RectI contentRect);
   
   static void initPersistFields();
   
public:
   DECLARE_CONOBJECT(VerbDisplay);
};


END_SW_NS
