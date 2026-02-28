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


ActorWalkState::ActorWalkState() : mWalkTarget(0,0), mAction(ACTION_IDLE), mWalkSpeed(0,0), mDirection(CostumeRenderer::SOUTH), mTieAxis(0)
{
  mWalkSpeed = Point2I(2,1);
  mPreferredAxis = 0;
   mNextBox = -1;
   
   mDebugSegment = Point2I(0,0);
   mDebugPoint = Point2I(0,0);
}

void ActorWalkState::updateTick(Actor& actor)
{
   CurrentAction prevAction = mAction;
   
   if (prevAction == ACTION_IDLE || prevAction == ACTION_FROZEN)
   {
      return;
   }
   
   Point2I delta = mWalkTarget - actor.getAnchorPosition();
   
   if (prevAction == ACTION_CHECK_MOVE ||
       delta == Point2I(0,0))
   {
      // See if we have a REAL walk target
      if (mWalkTarget != mRealWalkTarget)
      {
         adjustWalkTarget(actor);
         
         delta = mWalkTarget - actor.getAnchorPosition();
         
         S32 ax = std::abs(delta.x);
         S32 ay = std::abs(delta.y);
         
         mPreferredAxis = ax > ay + 5 ? 0 : 1;
         
         CostumeRenderer::DirectionValue curDir = dirFromDominantAxis(mPreferredAxis == 0 ? delta.x : delta.y, mPreferredAxis);
         if (curDir != mDirection)
         {
            actor.setDirection(curDir);
         }
         
      }
      else
      {
         mAction = ACTION_IDLE;
      }
   }
   
   if (mAction != ACTION_IDLE)
   {
      // Apply movement
      S32 step = clampStep(delta.x, mWalkSpeed.x);
      actor.mAnchor.x += step;
      step = clampStep(delta.y, mWalkSpeed.y);
      actor.mAnchor.y += step;
   }
   
   if (mAction != prevAction)
   {
      if (mAction == ACTION_IDLE)
      {
         actor.startAnim(actor.mStandAnim);
      }
      else if (mAction >= ACTION_MOVING)// && prevAction == ACTION_IDLE)
      {
         actor.startAnim(actor.mWalkAnim);
      }
   }
}

static Point2I NudgeToward(Point2I from, Point2I to, int pixels)
{
    int dx = to.x - from.x;
    int dy = to.y - from.y;

    if (dx == 0 && dy == 0)
        return from;

    float len = std::sqrt(float(dx * dx + dy * dy));
    if (len < 0.0001f)
        return from;

    float ux = dx / len;
    float uy = dy / len;

    return Point2I(
        int(std::lround(from.x + ux * pixels)),
        int(std::lround(from.y + uy * pixels))
    );
}

void ActorWalkState::adjustWalkTarget(Actor& actor)
{
   // re-adjusts walk target after hitting a new point
   
  if (actor.mAnchor != mRealWalkTarget)
  {
     Room* theRoom = dynamic_cast<Room*>(actor.getGroup());
     
     if (theRoom &&
         actor.mLastBox > 0 &&
         actor.mLastBox != mRealWalkTargetBox &&
         mRealWalkTargetBox > 0)
     {
        // If we trying to exit a box, assume we're already in the next box
        S32 actualBox = (mAction == ACTION_MOVING_TO_EXIT_PORTAL) ? mNextBox : actor.mLastBox;

        mNextBox = theRoom->mBoxes.getNextBoxIndex(actualBox, mRealWalkTargetBox);
        if (mNextBox == actualBox)
        {
           // Simply walk to the target location since we're in the last box
           mWalkTarget = mRealWalkTarget;
           mAction = ACTION_MOVING;
        }
        else if (mNextBox >= 0)
        {
           // Find the connecting edge in the TARGET box
           BoxInfo::PortalEdgePair pe = theRoom->mBoxes.FindBestPortalEdgePair(actualBox, mNextBox);

           Point2I P0;
           Point2I P1;
           Point2I portal;
           bool directWalk = false;
           
           if (theRoom->mBoxes.ComputeOverlapSegment(
                    pe.srcEdge.a, pe.srcEdge.b,
                    pe.dstEdge.a, pe.dstEdge.b,
                    P0, P1))
            {
               // If next box is actual target, move to that otherwise
               // try and aim for the center of the next box
               // (this allows for the character to follow a path around
               //  which doesn't keep in line with the target)
               if (mRealWalkTargetBox == mNextBox)
               {
                  bool inSlab1=true;
                  bool inSlab2=true;
                  Point2I actorPortal = theRoom->mBoxes.ClosestPointOnSegment(P0, P1, actor.mAnchor, &inSlab1);
                  Point2I targetPortal = theRoom->mBoxes.ClosestPointOnSegment(P0, P1, mRealWalkTarget, &inSlab2);
                  // If both points are inside the portal slab, just move directly to the target.
                  // TODO: probably a good idea to keep checking this during movement so we can switch to
                  // direct movement after some threshold.
                  if (inSlab1 && inSlab2)
                  {
                     portal = mRealWalkTarget;
                     directWalk = true;
                  }
                  else
                  {
                     portal = targetPortal;
                  }
               }
               else
               {
                  portal = theRoom->mBoxes.ClosestPointOnSegment(P0, P1, theRoom->mBoxes.GetBoxCenter(mNextBox));
               }
            }
            else
            {
                // fallback: midpoint between closest features
                portal = NudgeToward(pe.midOnSrc, theRoom->mBoxes.GetBoxCenter(actualBox), 2);
            }
           
           mDebugPoint = P0;
           mDebugSegment = P1;
           mWalkTarget = portal;

           
           if (mWalkTarget != mRealWalkTarget ||
               directWalk)
           {
              mAction = ACTION_MOVING_TO_EXIT_PORTAL;
           }
           else
           {
              mAction = ACTION_IDLE;
           }
        }
        else
        {
           // Something went wrong here, stop
           mWalkTarget = mRealWalkTarget;
           mAction = ACTION_IDLE;
        }
     }
     else
     {
        // Simply walk to the target location since we're in the box
        mWalkTarget = mRealWalkTarget;
        mAction = ACTION_MOVING;
     }
  }
  else
  {
     // We have arrived, stop
     mWalkTarget = mRealWalkTarget;
     mAction = ACTION_IDLE;
  }
}

// Actor

void Actor::initPersistFields()
{
   Parent::initPersistFields();

   addField("descName", TypeString, Offset(mDescName, Actor));

   initDisplayFields();
}

Actor::Actor()
{
  mCostume = nullptr;
  mTickCounter = 0;
  mTickSpeed = 4;
   mLayer = 0;
   mLastBox = -1;
   mTalking = false;
   
   mDisplayOffset = Point2I(0,0);

   mTalkParams = MessageDisplayParams();
   mTalkParams.messageOffset = Point2I(0,0);
   mTalkParams.tickSpeed = 4;
   mTalkParams.displayColor = (Color){255,255,255,255};
   mTalkParams.fontSize = 10;
   mTalkParams.lineSpacing = 2;
   mTalkParams.relative = true;
   mTalkParams.centered = true;
   
   mStandAnim = StringTable->insert("stand");
   mWalkAnim = StringTable->insert("walk");
   mStartTalkAnim = StringTable->insert("talkStart");
   mStopTalkAnim = StringTable->insert("talkStop");
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

void Actor::setPosition(Point2I pos)
{
   Room* room = dynamic_cast<Room*>(getGroup());
   if (room)
   {
      auto selectableFunc = +[](const BoxInfo::Box&){ return true; };
      BoxInfo::AdjustBoxResult result;
      if (room->mBoxes.FindNearestBoxAndSnapPoint(pos, selectableFunc, true, 0, result))
      {  
         // Found a good result, go for it!
         mAnchor = result.pos;
         mLastBox = result.box;
         mWalkState.mWalkTarget = mAnchor;
         mWalkState.mRealWalkTarget = result.pos;
         mWalkState.mRealWalkTargetBox = result.box;
      }
   }
   else
   {
      mAnchor = pos;
      mLastBox = -1;
      mWalkState.mWalkTarget = pos;
      mWalkState.mRealWalkTarget = pos;
      mWalkState.mRealWalkTargetBox = -1;
   }
   
   // NOTE: this re-calculates frame bounds in this case
   updateLayout(RectI(0,0,0,0));
}

void Actor::updateLayout(const RectI contentRect)
{
   // NOTE: this could be slightly off depending on offset at runtime, 
   // its acceptable for input.
   mLiveCostume.position = Point2F(mAnchor.x, mAnchor.y);
   RectI boundsRect = mLiveCostume.getCurrentBounds(mCostume->mState);
   resize(boundsRect.point, boundsRect.extent);
}

void Actor::setStanding()
{
   mWalkState.mAction = ActorWalkState::ACTION_IDLE;
   mWalkState.mWalkTarget = mWalkState.mRealWalkTarget = mAnchor;
   startAnim(mStandAnim);
}


void Actor::walkTo(Point2I pos)
{
   if (mWalkState.mAction == ActorWalkState::ACTION_FROZEN)
   {
      return;
   }
   
   // Determine real pos
   mWalkState.mWalkTarget = mAnchor;
   mWalkState.mRealWalkTarget = pos;
   mWalkState.mRealWalkTargetBox = -1;
   
   if (mWalkState.mAction == ActorWalkState::ACTION_IDLE)
   {
      mWalkState.mAction = ActorWalkState::ACTION_CHECK_MOVE;
   }
   
   Room* room = dynamic_cast<Room*>(getGroup());
   if (room)
   {
      auto selectableFunc = +[](const BoxInfo::Box&){ return true; };
      BoxInfo::AdjustBoxResult result;
      if (room->mBoxes.FindContainingBox(pos, selectableFunc, true, 0, result))
      {
         mWalkState.mRealWalkTarget = result.pos;
         mWalkState.mRealWalkTargetBox = result.box;
      }
      else if (room->mBoxes.FindNearestBoxAndSnapPoint(pos, selectableFunc, true, 0, result))
      {
         mWalkState.mRealWalkTarget = result.pos;
         mWalkState.mRealWalkTargetBox = result.box;
      }
   }
}

void Actor::onFixedTick(F32 dt)
{
  if (mTickCounter == 0)
  {
     if (mCostume)
     {
        ActorWalkState::CurrentAction lastAction = mWalkState.mAction;
        mWalkState.updateTick(*this);
        
        // Update box
        auto selectableFunc = +[](const BoxInfo::Box&){ return true; };
        Room* room = dynamic_cast<Room*>(getGroup());
        BoxInfo::AdjustBoxResult result;
        if (room->mBoxes.FindContainingBox(mAnchor, selectableFunc, true, 0, result) &&
            result.box >= 0)
        {
           mLayer = room->mBoxes.boxes[result.box].mask;
           mLastBox = result.box;
        }
        
        mLiveCostume.advanceTick(mCostume->mState);
        updateLayout(RectI(0,0,0,0));
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
     mLiveCostume.position = Point2F(mAnchor.x, mAnchor.y) + Point2F(mDisplayOffset.x, mDisplayOffset.y);
     //mLiveCostume.w
     mLiveCostume.render(mCostume->mState);
     
     // Draw debug stuff
     if (true)
     {
        ::DrawCircleLines(mWalkState.mWalkTarget.x, mWalkState.mWalkTarget.y, 2, RED);
        ::DrawCircleLines(mWalkState.mRealWalkTarget.x, mWalkState.mRealWalkTarget.y, 5, GREEN);
        
        ::DrawCircleLines(mWalkState.mDebugPoint.x, mWalkState.mDebugPoint.y, 10, YELLOW);
        ::DrawCircleLines(mWalkState.mDebugSegment.x, mWalkState.mDebugSegment.y, 7, PURPLE);
        
        Room* ourRoom = dynamic_cast<Room*>(getGroup());
        if (ourRoom)
        {
           if (mLastBox > 0)
           {
              BoxInfo::Box& box = ourRoom->mBoxes.boxes[mLastBox];
              if (box.numPoints >= 4)
              {
                 Point2I* points = ourRoom->mBoxes.points.data() + box.startPoint;
                 
                 Vector2 prevPoint = (Vector2){(float)points[0].x, (float)points[0].y};
                 Vector2 originPoint = prevPoint;
                 for (U32 i=1; i<box.numPoints; i++)
                 {
                    Vector2 curPoint = (Vector2){(float)points[i].x, (float)points[i].y};
                    ::DrawLineV(prevPoint, curPoint, (Color){255,0,255,255});
                    prevPoint = curPoint;
                 }
                 
                 ::DrawLineV(prevPoint, originPoint, (Color){255,0,255,255});
              }
           }
           
           if (mWalkState.mRealWalkTargetBox > 0)
           {
              BoxInfo::Box& box = ourRoom->mBoxes.boxes[mWalkState.mRealWalkTargetBox];
              if (box.numPoints >= 4)
              {
                 Point2I* points = ourRoom->mBoxes.points.data() + box.startPoint;
                 
                 Vector2 prevPoint = (Vector2){(float)points[0].x, (float)points[0].y};
                 Vector2 originPoint = prevPoint;
                 for (U32 i=1; i<box.numPoints; i++)
                 {
                    Vector2 curPoint = (Vector2){(float)points[i].x, (float)points[i].y};
                    ::DrawLineV(prevPoint, curPoint, (Color){0,0,255,255});
                    prevPoint = curPoint;
                 }
                 
                 ::DrawLineV(prevPoint, originPoint, (Color){0,0,255,255});
              }
           }
           
           if (mWalkState.mNextBox > 0)
           {
              BoxInfo::Box& box = ourRoom->mBoxes.boxes[mWalkState.mNextBox];
              if (box.numPoints >= 4)
              {
                 Point2I* points = ourRoom->mBoxes.points.data() + box.startPoint;
                 
                 Vector2 prevPoint = (Vector2){(float)points[0].x, (float)points[0].y};
                 Vector2 originPoint = prevPoint;
                 for (U32 i=1; i<box.numPoints; i++)
                 {
                    Vector2 curPoint = (Vector2){(float)points[i].x, (float)points[i].y};
                    ::DrawLineV(prevPoint, curPoint, (Color){255,0,255,255});
                    prevPoint = curPoint;
                 }
                 
                 ::DrawLineV(prevPoint, originPoint, (Color){255,0,255,255});
              }
           }
        }
     }

     RectI costumeBounds = mLiveCostume.getCurrentBounds(mCostume->mState);
     ::DrawRectangleLines(costumeBounds.point.x, costumeBounds.point.y, costumeBounds.extent.x, costumeBounds.extent.y, BLUE);
  }
}

void Actor::startAnim(StringTableEntry animName)
{
  mLiveCostume.setAnim(mCostume->mState, animName, mLiveCostume.curDirection);
}

void Actor::setDirection(CostumeRenderer::DirectionValue direction)
{
  mWalkState.mDirection = direction;
  if (mLiveCostume.curDirection != direction)
  {
     mLiveCostume.curDirection = direction;
     mLiveCostume.resetAnim(mCostume->mState, mLiveCostume.curDirection);
  }
}

void Actor::setCostume(SimWorld::Costume* costume)
{
  if (costume != mCostume)
  {
     mLiveCostume.init(costume->mState);
     mCostume = costume;
     mLiveCostume.setAnim(costume->mState, StringTable->insert("stand"), 1);
     mTickCounter = 0;
     mTalkParams.messageOffset = costume->mBaseTalkPos;
  }
}

void Actor::startTalk()
{
   mTalking = true;
   mLiveCostume.setAnim(mCostume->mState, mStartTalkAnim, mLiveCostume.curDirection);
}

void Actor::say(StringTableEntry msg)
{
   gGlobals.setActiveMessage(mTalkParams, this, nullptr, msg, true, 0);
}

void Actor::print(StringTableEntry msg)
{
   gGlobals.setActiveMessage(mTalkParams, this, nullptr, msg, false, 0);
}

void Actor::stopTalk()
{
   if (mTalking)
   {
      mLiveCostume.setAnim(mCostume->mState, mStopTalkAnim, mLiveCostume.curDirection);
      mTalking = false;
      
      if (mWalkState.mAction >= ActorWalkState::ACTION_MOVING)
      {
         startAnim(mWalkAnim);
      }
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
   object->startAnim(StringTable->insert(vmPtr->valueAsString(argv[2])));
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getWidth, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, isInBox, 3, 3, "")
{
   U32 boxId = vmPtr->valueAsInt(argv[2]);
   return KorkApi::ConsoleValue::makeUnsigned(object->mLastBox);
}

ConsoleMethodValue(Actor, say, 3, 3, "")
{
   StringTableEntry msg = vmPtr->valueAsString(argv[2]);
   object->say(msg);
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, putAt, 4, 4, "")
{
   Point2I destPoint(vmPtr->valueAsInt(argv[2]), vmPtr->valueAsInt(argv[3]));
   object->setPosition(destPoint);
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setDirection, 3, 3, "")
{
   object->setDirection((CostumeRenderer::DirectionValue)vmPtr->valueAsInt(argv[2]));
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, putAtObject, 2, 2, "")
{
   DisplayBase* targetObject = nullptr;
   if (Sim::findObject(argv[2], targetObject))
   {
      object->setPosition(targetObject->getHotSpot());
   }
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
   DisplayBase* targetObject = nullptr;
   if (Sim::findObject(argv[2], targetObject))
   {
      object->walkTo(targetObject->getHotSpot());
   }
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, print, 3, 3, "")
{
   StringTableEntry msg = vmPtr->valueAsString(argv[2]);
   object->print(msg);
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getScale, 2, 2, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, getState, 2, 2, "")
{
   return KorkApi::ConsoleValue::makeUnsigned(object->mWalkState.mAction);
}

ConsoleMethodValue(Actor, isMoving, 2, 2, "")
{
   return KorkApi::ConsoleValue::makeUnsigned(object->mWalkState.mAction > ActorWalkState::ACTION_IDLE);
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

ConsoleMethodValue(Actor, setStanding, 2, 2, "")
{
   object->setStanding();
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

ConsoleMethodValue(Actor, setTalkColor, 6, 6, "")
{
   object->mTalkParams.displayColor = (Color){(U8)vmPtr->valueAsInt(argv[2]), (U8)vmPtr->valueAsInt(argv[3]), (U8)vmPtr->valueAsInt(argv[4]), (U8)vmPtr->valueAsInt(argv[5])};
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
   object->mTalkParams.messageOffset = Point2I(vmPtr->valueAsInt(argv[2]), vmPtr->valueAsInt(argv[3]));
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setAnimVar, 4, 4, "")
{
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(Actor, setLayer, 3, 3, "")
{
   object->mLayer = vmPtr->valueAsInt(argv[2]);
   return KorkApi::ConsoleValue();
}

END_SW_NS
