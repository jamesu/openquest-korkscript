#pragma once

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS


class DisplayBase : public SimGroup
{
   typedef SimGroup Parent;
public:
   Point2I mPosition;
   Point2I mExtent;
   
   // NOTE: these are basically the style
   ColorI mBackColor;
   ColorI mColor;
   ColorI mHiColor;
   ColorI mDimColor;
   
   bool mCentered;
   bool mEnabled;
   U32 mHotKey;
   
   inline Point2I getPosition() const { return mPosition; }
   inline Point2I getExtent() const { return mExtent; }
   
   bool onAdd() override;
   void onRemove() override;
   
   virtual void resize(const Point2I newPosition, const Point2I newExtent);
   
   void forwardEvent(DBIEvent& event);
   
   virtual bool processInput(DBIEvent& event);
   
   void renderChildren(Point2I offset, RectI drawRect, Camera2D& globalCamera);
   
   virtual void onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera);
   
public:
   DECLARE_CONOBJECT(DisplayBase);
};


class RootUI : public DisplayBase
{
   typedef DisplayBase Parent;
public:
   static RootUI* sMainInstance;
   
   bool onAdd() override;
   
   void onRemove() override;
   
   void onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera) override;
   
   void setContent(DisplayBase* obj);
   
public:
   DECLARE_CONOBJECT(RootUI);
};


class ContainerDisplay : public DisplayBase
{
   typedef DisplayBase Parent;
public:
   DECLARE_CONOBJECT(ContainerDisplay);
};


END_SW_NS
