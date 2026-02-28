#pragma once

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS
struct RoomRender
{
   enum TransitionMode : U8
   {
      TRANSITION_NONE,
      TRANSITION_IRIS_IN,  // scissor towards center + shadow border, blank [then flip gfx after]
      TRANSITION_IRIS_OUT, // inverse scissor towards center + shadow border [no gfx change]
      TRANSITION_WIPE_HORIZONTAL_IN, // scissor horizontal
      TRANSITION_WIPE_HORIZONTAL_OUT, // scissor horizontal
      TRANSITION_WIPE_VERTICAL_IN, // scissor vertical
      TRANSITION_WIPE_VERTICAL_OUT // scissor vertical
   };
   
   enum TransitionWipeOrigin
   {
      WIPE_ORIG_LEFT,
      WIPE_ORIG_RIGHT,
      // aliases for above
      WIPE_ORIG_TOP = WIPE_ORIG_LEFT,
      WIPE_ORIG_BOTTOM = WIPE_ORIG_RIGHT
   };
   
   struct TransitionParams
   {
      U8 mode : 4;
      U8 params : 4;
   };
   
   struct TransitionInfo
   {
      TransitionParams data;
      F32 time;
   };
   
   enum
   {
       NumZPlanes = 3
   };
    TextureHandle backgroundImage;
    TextureHandle zPlanes[3];


   struct ObjectInfo
   {
      struct Entry
      {
         Point2I offset;
         TextureSlot* slot;
      };

      std::vector<Entry> images;
      std::vector<Entry> zPlanes[RoomRender::NumZPlanes];
      Point2I curRootPos;

      void reset()
      {
         curRootPos = Point2I(0,0);
         images.clear();
         for (U32 i=0; i<RoomRender::NumZPlanes; i++)
         {
            zPlanes[i].clear();
         }
      }
   };


    ObjectInfo objectInfo;
   
   Point2I roomDisplaySize;
   RectI backgroundSR;
   RectI clipRect;
   ColorI bgColor;
   
   TransitionInfo currentTransition;
   F32 transitionPos;
   
   bool mZPlanesDirty;
   bool transitionEnded;
   
   static float smoothstep(float t);

   static RectI computeWipeRect(int W, int H, float t01, TransitionMode mode, TransitionWipeOrigin origin);
   
   void updateTransition(float dt);

};



class Room : public DisplayBase, public ITickable
{
   typedef DisplayBase Parent;
public:

    StringTableEntry mImageFileName;
    StringTableEntry mBoxFileName;
    StringTableEntry mZPlaneFiles[RoomRender::NumZPlanes];

    U32 mTransFlags;
   
   RoomRender mRenderState;
   BoxInfo mBoxes;
   
   S32 findBoxContainingPoint(Point2I pos);
   Point2I projectPointOntoBox(Point2I pos, S32 box);
   
   Room();

   bool onAdd();
   
   void onRemove();

   void onEnter();
   void onLeave();
   
   void setTransitionMode(U8 mode, U8 param, F32 time, bool force=false);
   
   virtual void resize(const Point2I newPosition, const Point2I newExtent);
   virtual void updateLayout(const RectI contentRect);
   
   void updateResources();
   
   void updateZPlanes();
   
   virtual void onRender(Point2I offset, RectI drawRect, Camera2D& globalCam);
   
   virtual bool processInput(DBIEvent& event);
   virtual void onFixedTick(F32 dt);

   static void enterRoom(Room* room);
   static void leaveCurrentRoom();
   
   static void initPersistFields();

public:
   DECLARE_CONOBJECT(Room);
};


class RoomObject : public DisplayBase
{
   typedef DisplayBase Parent;
public:

   StringTableEntry mDescription;
   StringTableEntry mClassName;
   
   Direction mDirection;
   U32 mState;
   U32 mTransFlags;
   Point2I mHotspot; // from 
   
   // Parent dependency
   StringTableEntry mParentName;
   U32 mParentState;

   RoomObject();

   void updateResources();
   void updateLayout(const RectI contentRect);

   bool onAdd();
   
   void onRemove();

   virtual void onRender(Point2I offset, RectI drawRect, Camera2D& globalCam);

   // NOTE: object doesn't really render anything, it puts stuff into the z plane updates +
   // images.
   void enumerateRenderables(RoomRender::ObjectInfo& outState);

   static void initPersistFields();
   
public:
   DECLARE_CONOBJECT(RoomObject);
};

class RoomObjectState : public SimObject
{
   typedef SimObject Parent;
public:
   Point2I mHotSpot;
   Point2I mExtent;
   
   StringTableEntry mImageFileName;
   TextureHandle mTexture;
   StringTableEntry mZPlaneFiles[RoomRender::NumZPlanes];
   TextureHandle mZPlaneTextures[RoomRender::NumZPlanes];

   RoomObjectState();

   void updateResources();

   bool onAdd();
   
   void onRemove();

   void enumerateRenderables(RoomRender::ObjectInfo& outState);

   static void initPersistFields();
   
public:
   DECLARE_CONOBJECT(RoomObjectState);
};


END_SW_NS
