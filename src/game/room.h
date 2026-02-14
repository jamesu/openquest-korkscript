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
   
   enum
   {
       NumZPlanes = 3
   };
    TextureHandle backgroundImage;
    TextureHandle zPlanes[3];
   
   Point2I screenSize;
   RectI backgroundSR;
   RectI clipRect;
   ColorI bgColor;
   
   TransitionParams currentTransition;
   F32 transitionPos;
   F32 transitionTime;
   
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
   
   bool onAdd();
   
   void onRemove();
   
   void setTransitionMode(U8 mode, U8 param, F32 time);
   
   virtual void resize(const Point2I newPosition, const Point2I newExtent);
   
   void updateResources();
   
   virtual void onRender(Point2I offset, RectI drawRect, Camera2D& globalCam);
   
   
   virtual void onFixedTick(F32 dt);
   
   static void initPersistFields();

public:
   DECLARE_CONOBJECT(Room);
};


class RoomObject : public SimObject
{
   typedef SimObject Parent;
public:
   
   struct ObjectState
   {
      Point2I mOffset;
      StringTableEntry mImageName;
      TextureHandle mTexture;
      StringTableEntry mZPlaneName[RoomRender::NumZPlanes];
      TextureHandle mZPlaneTextures[RoomRender::NumZPlanes];
   };
   
   StringTableEntry mDescription;
   StringTableEntry mClassName;
   
   Point2I mPosition;
   Point2I mExtent;
   Point2I mHotspot;
   Direction mDirection;
   U32 mCurrentState;
   U32 mTransFlags;
   
   StringTableEntry mParentName;
   U32 mParentState;
   
   bool mActive;
   
   std::vector<ObjectState> mStates;
   
public:
   DECLARE_CONOBJECT(RoomObject);
};


END_SW_NS
