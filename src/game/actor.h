#pragma once

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS

// Class to manage actor walk state. This also handles moving
// and actor between walk boxes (via calculated portal nodes)
//
struct ActorWalkState
{
   enum CurrentAction
   {
      ACTION_FROZEN,
      ACTION_IDLE,                  // standing
      ACTION_CHECK_MOVE,            // calculate move target next tick
      ACTION_MOVING,                // move straight to target
      ACTION_MOVING_TO_EXIT_PORTAL  // when reached, moves to second portal node
   };
   // NOTE: state transitions are:
   // ACTION_IDLE -> ACTION_CHECK_MOVE
   // ACTION_CHECK_MOVE -> ACTION_MOVING
   // ACTION_CHECK_MOVE -> ACTION_MOVING_TO_EXIT_PORTAL
   // ACTION_MOVING_TO_EXIT_PORTAL -> ACTION_CHECK_EXIT_PORTAL
   // ACTION_CHECK_EXIT_PORTAL -> ACTION_MOVING_TO_EXIT_PORTAL2
   // ACTION_MOVING_TO_EXIT_PORTAL2 -> ACTION_CHECK_MOVE
   
   Point2I mRealWalkTarget; // Where we want to end up
   S32 mRealWalkTargetBox;  // Box where we want to end up
   
   // Where we are going at the moment
   Point2I mWalkTarget; // Current target
   Point2I mWalkSpeed;  //  x,y units per tick
   U8 mPreferredAxis;   // Direction we prefer to face when walking
   CostumeRenderer::DirectionValue mDirection; // this is calculated direction
   CurrentAction mAction; // Current walk state
   U32 mTieAxis;          // Which direction axis to pick if there is a tie
   
   Point2I mDebugSegment;
   Point2I mDebugPoint;
   
   S32 mNextBox; // Box we are currently trying to move into


   enum
   {
      MaxPath = 8
   };
   
   std::array<U8, MaxPath> mPathBoxes;
   U32 mPathBoxesLength;
   
   ActorWalkState();
   inline void reset();
   inline int sign(int x);
   inline S32 clampStep(S32 delta, S32 step);
   inline U32 pickAxis(Point2I delta);
   inline CostumeRenderer::DirectionValue dirFromDominantAxis(S32 value, U32 axis);
   
   void updateTick(Actor& actor);
   void adjustWalkTarget(Actor& actor);
};


inline void ActorWalkState::reset()
{
  mWalkTarget = Point2I(0,0);
  mRealWalkTarget = Point2I(0,0);
  mRealWalkTargetBox = -1;
  mWalkSpeed = Point2I(0,0);
  mDirection = CostumeRenderer::SOUTH;
  mAction = ACTION_IDLE;
  mTieAxis = 0;
}

inline int ActorWalkState::sign(int x) {
   return (x > 0) - (x < 0);
}

inline S32 ActorWalkState::clampStep(S32 delta, S32 step)
{
  return std::min<S32>(abs(delta), step) * sign(delta);
}

inline U32 ActorWalkState::pickAxis(Point2I delta)
{
  U32 axis = 0;
   delta.x = std::abs(delta.x);
   delta.y = std::abs(delta.y);
  
  if (delta.x == 0)
  {
     axis = 1;
  }
  else if (delta.y == 0)
  {
     axis = 0;
  }
  else
  {
     if (delta.x > delta.y)
     {
        axis = 0;
     }
     else if (delta.y > delta.x)
     {
        axis = 1;
     }
     else
     {
        axis = mTieAxis;
     }
  }
  
  return axis;
}

inline CostumeRenderer::DirectionValue ActorWalkState::dirFromDominantAxis(S32 value, U32 axis)
{
  static CostumeRenderer::DirectionValue lookupTable[] = {
     // x
     CostumeRenderer::WEST,
     CostumeRenderer::EAST,
     // y
     CostumeRenderer::NORTH,
     CostumeRenderer::SOUTH,
  };
  
  return lookupTable[(2 * axis) + (value > 0 ? 1 : 0)];
}

class Actor : public DisplayBase, public ITickable
{
   typedef DisplayBase Parent;
public:
   
   SimWorld::Costume* mCostume;
   CostumeRenderer::LiveState mLiveCostume;
   U16 mTickCounter;
   U16 mTickSpeed;
   U8 mLayer;
   S32 mLastBox;
   bool mTalking;
   
   ActorWalkState mWalkState;
   MessageDisplayParams mTalkParams;
   Point2I mDisplayOffset;
   
   StringTableEntry mStandAnim;
   StringTableEntry mWalkAnim;
   StringTableEntry mStartTalkAnim;
   StringTableEntry mStopTalkAnim;

   Actor();
   
   bool onAdd();
   void onRemove();
   
   void updateLayout(const RectI contentRect);
   
   void setPosition(Point2I pos);
   void setStanding();

   inline Point2I getTalkPos() const { return mTalkParams.messageOffset; }
   
   void walkTo(Point2I pos);
   
   virtual void onFixedTick(F32 dt);
   
   virtual void onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera);
   
   void startAnim(StringTableEntry animName);
   
   void setDirection(CostumeRenderer::DirectionValue direction);

   void setCostume(SimWorld::Costume* costume);

   static void initPersistFields();


   void startTalk();
   void stopTalk();

   void say(StringTableEntry msg);
   void print(StringTableEntry msg);
   
   
   DECLARE_CONOBJECT(Actor);
};


END_SW_NS
