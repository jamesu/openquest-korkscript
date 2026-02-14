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


bool DisplayBase::onAdd()
{
  return Parent::onAdd();
}

void DisplayBase::onRemove()
{
  Parent::onRemove();
}

void DisplayBase::resize(const Point2I newPosition, const Point2I newExtent)
{
  mPosition = newPosition;
  mExtent = newExtent;
  
  for (SimObject* obj : objectList)
  {
     DisplayBase* displayObj = dynamic_cast<DisplayBase*>(obj);
     if (displayObj)
     {
        displayObj->resize(newPosition, newExtent);
     }
  }
}

void DisplayBase::forwardEvent(DBIEvent& event)
{
  for (SimObject* obj : objectList)
  {
     DisplayBase* displayObj = dynamic_cast<DisplayBase*>(obj);
     if (displayObj)
     {
        displayObj->processInput(event);
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
        Point2I childPosition = offset + dObj->getPosition();
        RectI childClip(childPosition, dObj->getExtent());
        
        dObj->onRender(childPosition, childClip, globalCamera);
     }
  }
}

void DisplayBase::onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera)
{
}


ConsoleMethodValue(DisplayBase, setPosition, 4, 4, "")
{
   object->mPosition = Point2I(vmPtr->valueAsInt(argv[2]), vmPtr->valueAsInt(argv[3]));
   return KorkApi::ConsoleValue();
}

RootUI* RootUI::sMainInstance;

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

void RootUI::onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera)
{
   resize(offset, drawRect.extent);
   
   for (SimObject* obj : objectList)
   {
      DisplayBase* displayObj = dynamic_cast<DisplayBase*>(obj);
      if (displayObj)
      {
         displayObj->onRender(offset, drawRect, globalCamera);
      }
   }
}

ConsoleMethodValue(RootUI, setContent, 3, 3, "")
{
   DisplayBase* displayObject = nullptr;
   if (Sim::findObject(argv[2], displayObject))
   {
      object->addObject(displayObject);
   }
   
   return KorkApi::ConsoleValue();
}


END_SW_NS
