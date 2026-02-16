#include "engine.h"

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS


IMPLEMENT_CONOBJECT(Actor);


ActorWalkState::ActorWalkState() : mWalkTarget(0,0), mMoving(false), mWalkSpeed(0,0), mDirection(CostumeRenderer::SOUTH), mTieAxis(0)
{
  mWalkSpeed = Point2I(2,1);
  mPreferredAxis = 0;
}

void ActorWalkState::updateTick(Actor& actor)
{
   if (!mMoving)
   {
      return;
   }
   
   Point2I delta = mWalkTarget - actor.mPosition;
   
   if (delta == Point2I(0,0))
   {
      mMoving = false;
   }
   
   if (!mMoving)
   {
      return;
   }
   
   CostumeRenderer::DirectionValue curDir = dirFromDominantAxis(mPreferredAxis == 0 ? delta.x : delta.y, mPreferredAxis);
   if (curDir != mDirection)
   {
      actor.setDirection(curDir);
   }
   
   // Apply movement
   {
      S32 step = clampStep(delta.x, mWalkSpeed.x);
      actor.mPosition.x += step;
      step = clampStep(delta.y, mWalkSpeed.y);
      actor.mPosition.y += step;
   }
}

// Actor

Actor::Actor()
{
  mCostume = nullptr;
  mTickCounter = 0;
  mTickSpeed = 4;
}

bool Actor::onAdd()
{
  if (Parent::onAdd())
  {
     registerTickable();
     return true;
  }
  return false;
}

void Actor::onRemove()
{
  unregisterTickable();
}

void Actor::walkTo(Point2I pos)
{
  mWalkState.mWalkTarget = pos;
  if (mPosition != pos)
  {
     mWalkState.mPreferredAxis = mWalkState.pickAxis(pos - mPosition);
     mWalkState.mMoving = true;
     startAnim(StringTable->insert("walk"), false);
  }
}

void Actor::onFixedTick(F32 dt)
{
  if (mTickCounter == 0)
  {
     if (mCostume)
     {
        bool wasMoving = mWalkState.mMoving;
        mWalkState.updateTick(*this);
        if (mWalkState.mMoving != wasMoving)
        {
           startAnim(StringTable->insert("stand"), false);
        }
        mLiveCostume.advanceTick(mCostume->mState);
     }
  }
  
  mTickCounter++;
  if (mTickCounter >= mTickSpeed)
  {
     mTickCounter = 0;
  }
}

void Actor::onRender(Point2I offset, RectI drawRect, Camera2D& globalCamera)
{
  if (mCostume)
  {
     mLiveCostume.position = Point2F(offset.x, offset.y);
     //mLiveCostume.w
     mLiveCostume.render(mCostume->mState);
  }
}

void Actor::startAnim(StringTableEntry animName, bool isTalking)
{
  mLiveCostume.setAnim(mCostume->mState, animName, mLiveCostume.curDirection, false);
}

void Actor::setDirection(CostumeRenderer::DirectionValue direction)
{
  mWalkState.mDirection = direction;
  if (mLiveCostume.curDirection != direction)
  {
     mLiveCostume.curDirection = direction;
     mLiveCostume.resetAnim(mCostume->mState, mLiveCostume.curDirection, false);
  }
}

void Actor::setCostume(SimWorld::Costume* costume)
{
  if (costume != mCostume)
  {
     mLiveCostume.init(costume->mState);
     mCostume = costume;
     mLiveCostume.setAnim(costume->mState, StringTable->insert("stand"), 1, false);
     mTickCounter = 0;
  }
}


ConsoleMethodValue(Actor, setCostume, 3, 3, "")
{
   SimWorld::Costume* cost = nullptr;
   if (Sim::findObject(argv[2], cost))
   {
      object->setCostume(cost);
   }
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, animate, 3, 3, "")
{
   object->startAnim(StringTable->insert(vmPtr->valueAsString(argv[2])), false);
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, waitFor, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getWidth, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, isInBox, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, isMoving, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, say, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, putAt, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setDirection, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, putAtObject, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, walkTo, 4, 4, "")
{
   Point2I destPoint(vmPtr->valueAsInt(argv[2]), vmPtr->valueAsInt(argv[3]));
   object->walkTo(destPoint);
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, walkToObject, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, print, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getScale, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getLayer, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getFrame, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getAnimVar, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getAnimCounter, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setFrozen, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setTalkScript, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setStanding, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setIgnoreTurns, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setElevation, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setDefaultFrames, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setTalkColor, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setDescriptiveName, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setInitFrame, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setWidth, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setScale, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setZClip, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setIgnoreBoxes, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setAnimSpeed, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setTalkPos, 4, 4, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setAnimVar, 4, 4, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setLayer, 3, 3, "")
{
   return KorkApi::ConsoleValue();
}

END_SW_NS
