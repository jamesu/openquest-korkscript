#include "engine.h"

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS

IMPLEMENT_CONOBJECT(VerbDisplay);

VerbDisplay::VerbDisplay()
{
   mDisplayText = StringTable->EmptyString;
   mVerbName = StringTable->EmptyString;
   mRoomObject = nullptr;
}

bool VerbDisplay::onAdd()
{
   if (Parent::onAdd())
   {
      return true;
   }
   return false;
}

void VerbDisplay::onRemove()
{
   Parent::onRemove();
}

bool VerbDisplay::processInput(DBIEvent& event)
{
   return false;
}

void VerbDisplay::onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera)
{
   if (mRoomObject)
   {
      mRoomObject->onRender(offset, drawRect, globalCamera);
   }
   else if (mDisplayText && mDisplayText[0] != '\0')
   {
      if (mBackColor.a > 0)
      {
         DrawRectangle(offset.x, offset.y, mBounds.extent.x, mBounds.extent.y, mBackColor);
      }
      DrawText(mDisplayText, offset.x, offset.y, 12.0, mColor);
   }
}

void VerbDisplay::updateLayout(const RectI contentRect)
{
   if (mRoomObject)
   {
      mRoomObject->updateLayout(contentRect);
      resize(mAnchor, mRoomObject->mBounds.extent);
   }
}

void VerbDisplay::initPersistFields()
{
   Parent::initPersistFields();
   initDisplayFields();
   
   addField("roomObject", TypeSimObjectPtr, Offset(mRoomObject, VerbDisplay));
   addField("displayText", TypeString, Offset(mDisplayText, VerbDisplay));
}


END_SW_NS
