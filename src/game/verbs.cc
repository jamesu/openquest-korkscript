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
   mDisplayState = DEFAULT;
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


void VerbDisplay::onGainedCapture(DBIEvent& event)
{
   event.capturedControl = this;
   if (mDisplayState != DISABLED)
   {
      mDisplayState = HIGHLIGHTED;
   }
}

void VerbDisplay::onLostCapture(DBIEvent& event)
{
   event.capturedControl = nullptr;
   if (mDisplayState != DISABLED)
   {
      mDisplayState = DEFAULT;
   }
}

bool VerbDisplay::processInput(DBIEvent& event)
{
   if (!mEnabled)
   {
      if (event.capturedControl == this)
      {
         event.capturedControl->onLostCapture(event);
      }
      
      return false;
   }
   
   switch (event.type)
   {
      case UI_EVENT_MOUSE_MOVE:
      {
         if (mDisplayState != DISABLED)
         {
            const Point2I p = event.mouse.pos;
            const bool inside = mBounds.pointInRect(p);

            if (inside && event.capturedControl != this)
            {
               if (event.capturedControl)
               {
                  event.capturedControl->onLostCapture(event);
               }
               onGainedCapture(event);
            }
            else if (!inside && event.capturedControl == this)
            {
               onLostCapture(event);
            }

            mDisplayState = inside ? HIGHLIGHTED : DEFAULT;
         }

         return false;
      }
   }
   
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
   
   DrawRectangleLines(drawRect.point.x, drawRect.point.y, drawRect.extent.x, drawRect.extent.y, mDisplayState == HIGHLIGHTED ? GREEN : RED);
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
   addField("enabled", TypeBool, Offset(mEnabled, VerbDisplay));
}


END_SW_NS
