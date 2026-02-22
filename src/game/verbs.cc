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
      int textWidth = MeasureText(mDisplayText, mFontSize);
      int extra = mCentered ? (textWidth / 2) : 0;
      DrawText(mDisplayText, offset.x - extra, offset.y, mFontSize, mColor);
   }
   
   DrawRectangleLines(drawRect.point.x, drawRect.point.y, drawRect.extent.x, drawRect.extent.y, RED);
}

void VerbDisplay::updateLayout(const RectI contentRect)
{
   Point2I realStart = mAnchor;
   if (mRoomObject)
   {
      mRoomObject->updateLayout(contentRect);
      if (mCentered)
      {
         realStart -= mRoomObject->mBounds.extent / 2;
      }
      mMinContentSize = mRoomObject->mBounds.extent;
   }
   else if (mDisplayText && mDisplayText[0] != '\0')
   {
      int textWidth = MeasureText(mDisplayText, mFontSize);
      int extra = mCentered ? (textWidth / 2) : 0;
      realStart.x -= extra;
      mMinContentSize = Point2I(textWidth, std::ceil(mFontSize));
   }
   
   resize(realStart, mMinContentSize);
}

void VerbDisplay::initPersistFields()
{
   Parent::initPersistFields();
   initDisplayFields();
   
   addField("roomObject", TypeSimObjectPtr, Offset(mRoomObject, VerbDisplay));
   addField("displayText", TypeString, Offset(mDisplayText, VerbDisplay));
}


END_SW_NS
