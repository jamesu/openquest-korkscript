#pragma once

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS

struct DisplayPair
{
   Point2I tl;
   Point2I br;

   DisplayPair() {;}
};

class DisplayBase : public SimGroup
{
   typedef SimGroup Parent;
public:

   enum DisplayState : U8
   {
      DEFAULT,
      HIGHLIGHTED,
      DISABLED
   };

   RectI mBounds;   // Current control pos + extent
   Point2I mAnchor; // 
   Point2I mMinContentSize;
   Point2I mHotSpot;
   DisplayPair mMargin;
   DisplayPair mPadding;
   
   // NOTE: these are basically the style
   Color mBackColor;
   Color mColor;
   Color mHiColor;
   Color mDimColor;
   U32 mFontSize;
   
   bool mCentered;
   bool mEnabled;
   DisplayState mDisplayState;
   U32 mHotKey;

   static void initDisplayFields();

   DisplayBase();
   
   inline Point2I getAnchorPosition() const { return mAnchor; }
   inline Point2I getHotSpot() const { return mAnchor + mHotSpot; }
   inline Point2I getBoundedPosition() const { return mBounds.point; }
   inline Point2I getBoundedExtent() const { return mBounds.extent; }
   RectI getContentRect() const { return RectI(mPadding.tl, mBounds.extent - (mPadding.tl + mPadding.br)); }
   
   bool onAdd() override;
   void onRemove() override;

   virtual void onGainedCapture(DBIEvent& event);
   virtual void onLostCapture(DBIEvent& event);
   
   virtual void resize(const Point2I newPosition, const Point2I newExtent);
   virtual void updateLayout(const RectI contentRect);
   
   virtual void setPosition(Point2I newPosition);
   
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
   
   RootUI();
   
   bool onAdd() override;
   
   void onRemove() override;
   
   bool processInput(DBIEvent& event) override;
   
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
