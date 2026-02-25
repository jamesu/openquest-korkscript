#include "engine.h"

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS


IMPLEMENT_CONOBJECT(DisplayBase);
IMPLEMENT_CONOBJECT(RootUI);
IMPLEMENT_CONOBJECT(ContainerDisplay);


void DisplayBase::initDisplayFields()
{
   addField("anchorPoint", TypePoint2I, Offset(mAnchor, DisplayBase));
   addField("hotSpot", TypePoint2I, Offset(mHotSpot, DisplayBase));
   addField("marginTL", TypePoint2I, Offset(mMargin.tl, DisplayBase));
   addField("marginBR", TypePoint2I, Offset(mMargin.br, DisplayBase));
   addField("paddingTL", TypePoint2I, Offset(mPadding.tl, DisplayBase));
   addField("paddingBR", TypePoint2I, Offset(mPadding.br, DisplayBase));
   addField("center", TypeBool, Offset(mCentered, DisplayBase));
   addField("fontSize", TypeS32, Offset(mFontSize, DisplayBase));
}

DisplayBase::DisplayBase()
{
   mBounds = RectI(0,0,0,0);
   mAnchor = Point2I(0,0);
   mMinContentSize = Point2I(0,0);
   mBackColor = (Color){0,0,0,0};
   mColor = (Color){255,255,255,255};
   mHiColor = (Color){0,0,0,255};
   mDimColor = (Color){0,0,0,255};
   mHotSpot = Point2I(0,0);
   mCentered = false;
   mEnabled = true;
   mHotKey = 0;
   mDisplayState = DEFAULT;
   mFontSize = 10;
}

bool DisplayBase::onAdd()
{
  return Parent::onAdd();
}

void DisplayBase::onRemove()
{
  Parent::onRemove();
}

void DisplayBase::onGainedCapture(DBIEvent& event)
{

}

void DisplayBase::onLostCapture(DBIEvent& event)
{

}

void DisplayBase::updateLayout(const RectI contentRect)
{
   // NOTE: default layout; only TL margin used.
   for (SimObject* obj : objectList)
   {
      DisplayBase* displayObj = dynamic_cast<DisplayBase*>(obj);
      if (displayObj)
      {
         // Pos
         Point2I childPos = contentRect.point + displayObj->mAnchor; // base
         childPos += displayObj->mMargin.tl;     // + margin
         // Extent
         Point2I childSize = displayObj->mPadding.tl + displayObj->mMinContentSize;
         childSize += displayObj->mPadding.br;
         
         displayObj->resize(childPos, childSize);
         
         // Update layout in child
         childSize = displayObj->mBounds.extent - (displayObj->mPadding.tl + displayObj->mPadding.br);
         RectI childContent(displayObj->mPadding.tl, childSize);
         displayObj->updateLayout(childContent);
      }
   }
}

void DisplayBase::resize(const Point2I newPosition, const Point2I newExtent)
{
   // NOTE: unlike torque, doesn't infer interior layout change
   mBounds.point = newPosition;
   mBounds.extent = newExtent;
   // Make sure we don't go negative on extent
   mBounds.extent.x = std::max<S32>(mBounds.extent.x, mMinContentSize.x + mPadding.tl.x + mPadding.tl.x);
   mBounds.extent.y = std::max<S32>(mBounds.extent.y, mMinContentSize.y + mPadding.tl.y + mPadding.tl.y);
}

void DisplayBase::setPosition(Point2I newPosition)
{
   resize(newPosition, mBounds.extent);
}

void DisplayBase::forwardEvent(DBIEvent& event)
{
  for (SimObject* obj : objectList)
  {
     DisplayBase* displayObj = dynamic_cast<DisplayBase*>(obj);
     if (displayObj)
     {
        displayObj->processInput(event);
        if (event.handled)
        {
           break;
        }
     }
  }
}

bool DisplayBase::processInput(DBIEvent& event)
{
  return false;
}

void DisplayBase::renderChildren(Point2I offset, RectI drawRect, Camera2D& globalCamera)
{
  for (SimObject* obj : objectList)
  {
     DisplayBase* dObj = dynamic_cast<DisplayBase*>(obj);
     if (dObj)
     {
        Point2I childPosition = dObj->getAnchorPosition();
        RectI childClip(dObj->getBoundedPosition(), dObj->getBoundedExtent());
        
        if (childClip.intersect(drawRect))
        {
           dObj->onRender(childPosition, childClip, globalCamera);
        }
     }
  }
}

void DisplayBase::onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera)
{
}


ConsoleMethodValue(DisplayBase, setPosition, 4, 4, "")
{
   object->setPosition(Point2I(vmPtr->valueAsInt(argv[2]), vmPtr->valueAsInt(argv[3])));
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(DisplayBase, updateLayout, 2, 2, "")
{
   object->updateLayout(object->getContentRect());
   return KorkApi::ConsoleValue();
}

RootUI* RootUI::sMainInstance;

RootUI::RootUI()
{
}

bool RootUI::onAdd()
{
  if (Parent::onAdd())
  {
     sMainInstance = this;
     return true;
  }
  return false;
}

void RootUI::onRemove()
{
  if (sMainInstance == this)
  {
     sMainInstance = nullptr;
  }
  
  Parent::onRemove();
}

bool RootUI::processInput(DBIEvent& event)
{
   if (event.capturedControl && 
      event.type >= UI_EVENT_MOUSE_MOVE && event.type <= UI_EVENT_MOUSE_WHEEL) 
   {
      event.capturedControl->processInput(event);
   }
   else
   {
      forwardEvent(event);
   }
}

void RootUI::onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera)
{
   resize(offset, drawRect.extent);
   
   renderChildren(offset, drawRect, globalCamera);
}

void RootUI::setContent(DisplayBase* obj)
{
   auto itr = std::find(objectList.begin(), objectList.end(), obj);
   if (itr == objectList.end())
   {
      clear();
      addObject(obj);
   }
}

ConsoleMethodValue(RootUI, setContent, 3, 3, "")
{
   DisplayBase* displayObject = nullptr;
   if (Sim::findObject(argv[2], displayObject))
   {
      object->setContent(displayObject);
   }
   
   return KorkApi::ConsoleValue();
}


END_SW_NS
