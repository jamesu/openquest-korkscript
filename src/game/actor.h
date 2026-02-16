#pragma once

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS


struct ActorWalkState
{
   Point2I mWalkTarget;
   Point2I mWalkSpeed; // x,y units per tick
   U8 mPreferredAxis;
   CostumeRenderer::DirectionValue mDirection; // this is calculated direction
   bool mMoving;
   U32 mTieAxis;
   
   ActorWalkState();
   inline void reset();
   inline int sign(int x);
   inline S32 clampStep(S32 delta, S32 step);
   inline U32 pickAxis(Point2I delta);
   inline CostumeRenderer::DirectionValue dirFromDominantAxis(S32 value, U32 axis);
   
   void updateTick(Actor& actor);
};


inline void ActorWalkState::reset()
{
  mWalkTarget = Point2I(0,0);
  mWalkSpeed = Point2I(0,0);
  mDirection = CostumeRenderer::SOUTH;
  mMoving = false;
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
   
   ActorWalkState mWalkState;
   
   Actor();
   
   bool onAdd();
   void onRemove();
   
   void walkTo(Point2I pos);
   
   virtual void onFixedTick(F32 dt);
   
   virtual void onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera);
   
   void startAnim(StringTableEntry animName, bool isTalking);
   
   void setDirection(CostumeRenderer::DirectionValue direction);

   void setCostume(SimWorld::Costume* costume);
   
   
   DECLARE_CONOBJECT(Actor);
};


END_SW_NS
