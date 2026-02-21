#include "engine.h"

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS


IMPLEMENT_CONOBJECT(Room);
IMPLEMENT_CONOBJECT(RoomObject);
IMPLEMENT_CONOBJECT(RoomObjectState);


float RoomRender::smoothstep(float t) {
    // clamp
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    return t*t*(3.0f - 2.0f*t);
}

RectI RoomRender::computeWipeRect(int W, int H, float t01, TransitionMode mode, TransitionWipeOrigin origin)
{
    float p = smoothstep(t01);
    float s = (mode == TRANSITION_WIPE_HORIZONTAL_IN ||
               mode == TRANSITION_WIPE_VERTICAL_IN) ? p : (1.0f - p);  // size fraction: grows for IN, shrinks for OUT

    float x = 0, y = 0, w = (float)W, h = (float)H;

    if (mode == TRANSITION_WIPE_HORIZONTAL_IN || mode == TRANSITION_WIPE_HORIZONTAL_OUT) {
        w = s * W;

        if (origin == WIPE_ORIG_LEFT) {
            x = 0;
        } else { // WIPE_ORIG_RIGHT
            x = W - w;
        }
        y = 0; h = H;
    } else { // AXIS_V
        h = s * H;

        if (origin == WIPE_ORIG_TOP) {
            y = 0;
        } else { // WIPE_ORIG_BOTTOM
            y = H - h;
        }
        x = 0; w = W;
    }

    // Avoid negative/NaN and tiny seams: clamp + round outward a bit
    if (w < 0) w = 0; if (h < 0) h = 0;
    if (x < 0) x = 0; if (y < 0) y = 0;
    if (x + w > W) w = W - x;
    if (y + h > H) h = H - y;

    return RectI(x,y,w,h);
}

void RoomRender::updateTransition(float dt)
{
   transitionPos += dt * (1.0/transitionTime);
   
   if (transitionPos > 1.0)
   {
      transitionPos = 1.0;
   }
   
   Point2F halfSize = Point2F(screenSize.x, screenSize.y) * 0.5f;
   Point2I fullSize = Point2I(screenSize.x, screenSize.y);
   
   switch (currentTransition.mode)
   {
      case TRANSITION_IRIS_IN:
         clipRect.point = (halfSize * transitionPos).asPoint2I();
         clipRect.extent = (fullSize - clipRect.point) - (halfSize * transitionPos).asPoint2I();
         break;
      case TRANSITION_IRIS_OUT:
         clipRect.point = (halfSize * (1.0f-transitionPos)).asPoint2I();
         clipRect.extent = (fullSize - clipRect.point) - (halfSize * (1.0f-transitionPos)).asPoint2I();
         break;
      case TRANSITION_WIPE_HORIZONTAL_IN:
      case TRANSITION_WIPE_HORIZONTAL_OUT:
      case TRANSITION_WIPE_VERTICAL_IN:
      case TRANSITION_WIPE_VERTICAL_OUT:
         clipRect = computeWipeRect(screenSize.x,
                                    screenSize.y,
                                    transitionPos,
                                    (TransitionMode)currentTransition.mode,
                                    (TransitionWipeOrigin)currentTransition.params);
         break;
      default:
         clipRect = RectI(0,0,screenSize.x, screenSize.y);
         break;
         
   }
}

Room::Room()
{
   mImageFileName ="";
   mBoxFileName = "";
   mTransFlags = 0;

   for (U32 i=0; i<RoomRender::NumZPlanes; i++)
   {
      mZPlaneFiles[i] = "";
   }
}

bool Room::onAdd()
{
   if (Parent::onAdd())
   {
      registerTickable();
      
      mMinContentSize = Point2I(320, 200);
      
      if (mBoxFileName && mBoxFileName[0] != '\0')
      {
         mBoxes.reset();
         FileStream fs;
         if (fs.open(mBoxFileName, FileStream::Read))
         {
            mBoxes.read(fs);
         }
      }
      
      return true;
   }
   return false;
}

void Room::onRemove()
{
   unregisterTickable();
}

void Room::setTransitionMode(U8 mode, U8 param, F32 time)
{
   mRenderState.transitionPos = 0.0f;
   mRenderState.transitionTime = time;
   mRenderState.currentTransition.mode = mode;
   mRenderState.currentTransition.params = param;
}

void Room::resize(const Point2I newPosition, const Point2I newExtent)
{
   mBounds.point = newPosition;
   mBounds.extent = newExtent;
   mRenderState.screenSize.x = mBounds.extent.x;
   mRenderState.screenSize.y = mBounds.extent.y;
}

void Room::updateLayout(const RectI contentRect)
{
   // NO padding
   mPadding.tl = Point2I(0,0);
   mPadding.br = Point2I(0,0);
   
   Parent::updateLayout(contentRect);
}

void Room::updateResources()
{
   mRenderState.backgroundImage = mImageFileName && *mImageFileName ? gTextureManager->loadTexture(mImageFileName) : nullptr;
   for (uint32_t i=0; i<RoomRender::NumZPlanes; i++)
   {
      mRenderState.zPlanes[i] = mZPlaneFiles[i] && *mZPlaneFiles[i] ? gTextureManager->loadTexture(mZPlaneFiles[i]) : nullptr;
   }
   
   TextureSlot* slot = gTextureManager->resolveHandle(mRenderState.backgroundImage);
   if (slot)
   {
      mRenderState.backgroundSR = RectI(Point2I(0,0), Point2I(slot->mTexture.width, slot->mTexture.height));
   }
   
   mRenderState.mZPlanesDirty = true;
}

void Room::updateZPlanes()
{
   Vector2 origin = { 0.0, 0.0 };

   for (U32 zPlane=0; zPlane<RoomRender::NumZPlanes; zPlane++)
   {
      BeginTextureMode(gGlobals.roomZPlaneRt[zPlane]);

      // Draw room zplane
      {
         TextureSlot* maskSlot = gTextureManager->resolveHandle(mRenderState.zPlanes[zPlane]);
         if (maskSlot)
         {
            Rectangle src = { 0.0f, 0.0f, (float)maskSlot->mTexture.width, (float)maskSlot->mTexture.height };
            Rectangle dest = { 0.0f, 0.0f, (float)maskSlot->mTexture.width, (float)maskSlot->mTexture.height };
            DrawTexturePro(maskSlot->mTexture, src, dest, origin, 0.0f, WHITE);
         }
               

      }

      // Draw all active object z planes
      {
         // 
         for (RoomRender::ObjectInfo::Entry& e : mRenderState.objectInfo.zPlanes[zPlane])
         {
            if (e.slot)
            {
               Rectangle src = { 0.0f, 0.0f, (float)e.slot->mTexture.width, (float)e.slot->mTexture.height };
               Rectangle dest = { (float)e.offset.x, (float)e.offset.y, (float)e.slot->mTexture.width, (float)e.slot->mTexture.height };
               DrawTexturePro(e.slot->mTexture, src, dest, origin, 0.0f, WHITE);
            }
         }
      }

      EndTextureMode();
   }
}

void Room::onRender(Point2I offset, RectI drawRect, Camera2D& globalCam)
{
   if (mRenderState.backgroundImage.getNum() == 0)
   {
      updateResources();
   }
   
   // We need to maintain the original aspect ratio of the scene when drawing the image in respect to the scumm coord system
   
   Rectangle source = { (float)mRenderState.backgroundSR.point.x, (float)mRenderState.backgroundSR.point.y, (float)mRenderState.backgroundSR.extent.x, (float)mRenderState.backgroundSR.extent.y };
   Rectangle dest = { (float)offset.x, (float)offset.y, (float)drawRect.extent.x, (float)drawRect.extent.y };
   Vector2 origin = { 0.0, 0.0 };

   // Update object state
   mRenderState.objectInfo.reset();
   for (SimObject* obj : objectList)
   {
      RoomObject* roomObj = dynamic_cast<RoomObject*>(obj);
      if (roomObj)
      {
         roomObj->enumerateRenderables(mRenderState.objectInfo);
      }
   }
   
   // z planes need to be kept current; these are handled by copying
   // the base planes + object planes to mask textures.
   if (mRenderState.mZPlanesDirty || true)
   {
      updateZPlanes();
   }
   
   Camera2D localCamera = {};
   BeginTextureMode(gGlobals.roomRt);
   {
      DrawRectangle(0, 0, 300, 200, (Color){255,0,0,255});
      
      TextureSlot* slot = gTextureManager->resolveHandle(mRenderState.backgroundImage);
      if (slot)
      {
         Rectangle localDest = { 0.0f, 0.0f, (float)slot->mTexture.width, (float)slot->mTexture.height };
         DrawTexturePro(slot->mTexture, source, localDest, origin, 0.0f, WHITE);
      }

      // Draw the objects
      for (SimObject* obj : objectList)
      {
         RoomObject* roomObj = dynamic_cast<RoomObject*>(obj);
         if (roomObj)
         {
            roomObj->updateLayout(getContentRect());

            // NOTE: room objects update their layout in this case
            Point2I childPosition = roomObj->getAnchorPosition();
            RectI childClip(roomObj->getBoundedPosition(), roomObj->getBoundedExtent());
            roomObj->onRender(childPosition, childClip, localCamera);
         }
      }
      
      Point2F scalingFactor = Point2F(mRenderState.screenSize.x / 320.0f, mRenderState.screenSize.y / 200.0f);
      
      for (BoxInfo::Box& box : mBoxes.boxes)
      {
         if (box.numPoints >= 4)
         {
            Point2I* points = mBoxes.points.data() + box.startPoint;
            
            Vector2 prevPoint = (Vector2){(float)points[0].x, (float)points[0].y};
            Vector2 originPoint = prevPoint;
            for (U32 i=1; i<box.numPoints; i++)
            {
               Vector2 curPoint = (Vector2){(float)points[i].x, (float)points[i].y};
               ::DrawLineV(prevPoint, curPoint, (Color){255,255,255,255});
               prevPoint = curPoint;
            }
            
            ::DrawLineV(prevPoint, originPoint, (Color){255,255,255,255});
         }
      }
      
      // Ok now draw the layers
      std::vector<Actor*> sortedActors;
      sortedActors.reserve(objectList.size());
      
      for (SimObject* obj : objectList)
      {
         Actor* actor = dynamic_cast<Actor*>(obj);
         if (actor)
            sortedActors.push_back(actor);
      }
      
      std::sort(sortedActors.begin(), sortedActors.end(), [](const Actor* a, const Actor* b){
         if (a->mBounds.point.y < b->mBounds.point.y)
         {
            return true;
         }
         
         if (a->getId() < b->getId())
         {
            return true;
         }
         
         return false;
      });
      
      // Actors need to be masked by the current z planes.
      // these need to be kept current
      U32 lastActor = 0;
      int locMask = GetShaderLocation(gGlobals.shaderMask, "maskTex");
      int locRtSize = GetShaderLocation(gGlobals.shaderMask, "rtSizePx");
      int locOff    = GetShaderLocation(gGlobals.shaderMask, "roomOffsetPx");
      int locRoomSz = GetShaderLocation(gGlobals.shaderMask, "roomSizePx");
      
      Vector2 rtSize   = { (float)gGlobals.roomRt.texture.width, (float)gGlobals.roomRt.texture.height }; // 320,200
      Vector2 roomOff  = { 0.0f, 0.0f };
      Vector2 roomSize = { 320.0f, 144.0f };
      
      for (U32 zPlane=0; zPlane<RoomRender::NumZPlanes; zPlane++)
      {
         // Start drawing using the zPlane RT as a mask
         BeginBlendMode(BLEND_ALPHA);
         BeginShaderMode(gGlobals.shaderMask);

         Texture2D& tex = gGlobals.roomZPlaneRt[zPlane].texture; 
         
         SetTextureFilter(tex, TEXTURE_FILTER_POINT);
         SetTextureWrap(tex, TEXTURE_WRAP_CLAMP);
         
         SetShaderValueTexture(gGlobals.shaderMask, locMask, tex);
         
         SetShaderValue(gGlobals.shaderMask, locRtSize, &rtSize, SHADER_UNIFORM_VEC2);
         SetShaderValue(gGlobals.shaderMask, locOff,    &roomOff, SHADER_UNIFORM_VEC2);
         SetShaderValue(gGlobals.shaderMask, locRoomSz, &roomSize, SHADER_UNIFORM_VEC2);
      
         
         for (Actor* obj : sortedActors)
         {
            Actor* actor = dynamic_cast<Actor*>(obj);
            if (actor && actor->mLayer == zPlane+1)
            {
               // NOTE: actors can have costume parts all over the place,
               // so we just use the rooms clip rect here.
               Point2I childPosition = actor->getAnchorPosition();
               RectI childClip(actor->getBoundedPosition(), actor->getBoundedExtent());
               actor->onRender(childPosition, childClip, localCamera);
            }
         }
         
         EndBlendMode();
         EndShaderMode();
         
         //break;
      }
      
      // Draw layer 0 on top
      for (Actor* obj : sortedActors)
      {
         Actor* actor = dynamic_cast<Actor*>(obj);
         if (actor && actor->mLayer == 0)
         {
            Point2I childPosition = actor->getAnchorPosition();
            RectI childClip(actor->getBoundedPosition(), actor->getBoundedExtent());
            actor->onRender(childPosition, childClip, localCamera);
         }
      }
      
   }
   EndTextureMode();
   
   // Now render the room to the canvas
   
   // Transform
   RectI globalRect = WorldRectToScreen(mRenderState.clipRect, globalCam);
   //BeginScissorMode(globalRect.point.x, globalRect.point.y, globalRect.extent.x, globalRect.extent.y);
   
   DrawTexturePro(gGlobals.roomRt.texture,
                  (Rectangle){0,0,320,-200},
                  (Rectangle){(float)globalRect.point.x, (float)globalRect.point.y, (float)globalRect.extent.x, (float)globalRect.extent.y},
                  origin, 0, WHITE);
   
   //EndScissorMode();
   
}

void Room::onFixedTick(F32 dt)
{
   mRenderState.updateTransition(dt);
}

void Room::initPersistFields()
{
   Parent::initPersistFields();
   
   addField("image", TypeString, Offset(mImageFileName, Room));
   addField("boxFile", TypeString, Offset(mBoxFileName, Room));
   addField("zPlane", TypeString, Offset(mZPlaneFiles, Room), RoomRender::NumZPlanes);
}


void Room::onEnter()
{
   RootUI::sMainInstance->setContent(this);
}

void Room::onLeave()
{
   RootUI::sMainInstance->removeObject(this);
}


void Room::enterRoom(Room* room)
{
   if (gGlobals.currentRoom)
   {
      leaveCurrentRoom();
   }

   gGlobals.currentRoom = room;
   room->onEnter();
}

void Room::leaveCurrentRoom()
{
   if (gGlobals.currentRoom)
   {
      gGlobals.currentRoom->onLeave();
   }
   gGlobals.currentRoom = nullptr;
}

ConsoleMethodValue(Room, setTransitionMode, 5, 5, "mode, param, time")
{
   object->setTransitionMode(vmPtr->valueAsInt(argv[2]), vmPtr->valueAsInt(argv[3]), vmPtr->valueAsFloat(argv[4]));
   return KorkApi::ConsoleValue();
}


RoomObject::RoomObject()
{
   mState = 1;
   mTransFlags = 0;
}

void RoomObject::updateResources()
{
   for (SimObject* obj : objectList)
   {
      RoomObjectState* state = dynamic_cast<RoomObjectState*>(obj);
      if (state)
      {
         state->updateResources();
      }
   }
}

bool RoomObject::onAdd()
{
   if (Parent::onAdd())
   {
      return true;
   }
   return false;
}

void RoomObject::onRemove()
{
   Parent::onRemove();
}

void RoomObject::onRender(Point2I offset, RectI drawRect, Camera2D& globalCam)
{
   if (mState == 0)
   {
      return;
   }

   U32 curStateIndex = mState-1;
   if (curStateIndex < objectList.size())
   {
      Vector2 origin = { 0.0, 0.0 };
      RoomObjectState* curState = dynamic_cast<RoomObjectState*>(objectList[curStateIndex]);
      offset += curState->mOffset;
      
      TextureSlot* slot = gTextureManager->resolveHandle(curState->mTexture);
      if (slot)
      {
         Rectangle src = { 0.0f, 0.0f, (float)slot->mTexture.width, (float)slot->mTexture.height };
         Rectangle dest = { (float)offset.x, (float)offset.y, (float)slot->mTexture.width, (float)slot->mTexture.height };
         DrawTexturePro(slot->mTexture, src, dest, origin, 0.0f, WHITE);
      }
   }
}

void RoomObject::enumerateRenderables(RoomRender::ObjectInfo& outState)
{
   if (mState == 0)
   {
      return;
   }

   U32 curStateIndex = mState-1;
   if (curStateIndex < objectList.size())
   {
      RoomObjectState* curState = dynamic_cast<RoomObjectState*>(objectList[curStateIndex]);
      if (curState)
      {
         outState.curRootPos = mAnchor;
         curState->enumerateRenderables(outState);
      }
   }
}

void RoomObject::initPersistFields()
{
   Parent::initPersistFields();
   
   registerClassNameFields(false);
   
   addField("descName", TypeString, Offset(mDescription, RoomObject));
   addField("state", TypeS32, Offset(mState, RoomObject));
   addField("dir", TypeS32, Offset(mDirection, RoomObject));
   addField("trans", TypeS32, Offset(mTransFlags, RoomObject));
   addField("hotSpot", TypePoint2I, Offset(mHotspot, RoomObject));
}

RoomObjectState::RoomObjectState()
{
   mOffset = Point2I(0,0);
   mImageFileName = "";
   mTexture = TextureHandle();
   for (U32 i=0; i<RoomRender::NumZPlanes; i++)
   {
      mZPlaneFiles[i] = "";
      mZPlaneTextures[i] = TextureHandle();
   }
}

void RoomObjectState::updateResources()
{
   if (mImageFileName && mImageFileName[0] != '\0')
   {
      mTexture = gTextureManager->loadTexture(mImageFileName);
   }

   for (U32 i=0; i<RoomRender::NumZPlanes; i++)
   {
      if (mZPlaneFiles[i] && !mZPlaneFiles[0] != '\0')
      {
         mZPlaneTextures[i] = gTextureManager->loadTexture(mZPlaneFiles[i]);
      }
      else
      {
         mZPlaneTextures[i] = TextureHandle();
      }
   }
}

bool RoomObjectState::onAdd()
{
   if (Parent::onAdd())
   {
      updateResources();
      return true;
   }

   return false;
}

void RoomObjectState::onRemove()
{

}

void RoomObjectState::enumerateRenderables(RoomRender::ObjectInfo& outState)
{
   RoomRender::ObjectInfo::Entry outInfo;
   outInfo.offset = outState.curRootPos + mOffset;

   TextureSlot* slot = nullptr;

   slot = gTextureManager->resolveHandle(mTexture);
   if (slot)
   {
      outInfo.slot = slot;
      outState.images.push_back(outInfo);
   }

   for (U32 i=0; i<RoomRender::NumZPlanes; i++)
   {
      slot = gTextureManager->resolveHandle(mZPlaneTextures[i]);
      if (slot)
      {
         outInfo.slot = slot;
         outState.zPlanes[i].push_back(outInfo);
      }
   }
}

void RoomObjectState::initPersistFields()
{
   Parent::initPersistFields();

   addField("offset", TypePoint2I, Offset(mOffset, RoomObjectState));
   addField("image", TypeString, Offset(mImageFileName, RoomObjectState));
   addField("zPlane", TypeString, Offset(mZPlaneFiles, RoomObjectState), RoomRender::NumZPlanes);
}

ConsoleFunctionValue(screenEffect, 2, 2, "")
{
   if (gGlobals.currentRoom)
   {
      U32 code = vmPtr->valueAsInt(argv[1]);
      gGlobals.currentRoom->setTransitionMode(code & 0xFF, (code >> 8) & 0xFF, 1.0f);
   }
   
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(startRoom, 2, 2, "")
{
   Room* roomObject = nullptr;
   if (Sim::findObject(argv[1], roomObject))
   {
      Room::enterRoom(roomObject);
   }
   
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(startRoomWithEgo, 4, 4, "")
{
   Room* roomObject = nullptr;
   if (Sim::findObject(argv[1], roomObject))
   {
      if (gGlobals.currentEgo)
      {
         roomObject->addObject(gGlobals.currentEgo);
         gGlobals.currentEgo->setPosition(Point2I(vmPtr->valueAsInt(argv[2]),
                                                  vmPtr->valueAsInt(argv[3])));
      }
      Room::enterRoom(roomObject);
   }
   
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(putActorAt, 5, 5, "")
{
   Actor* actorObject = nullptr;
   Room* roomObject = nullptr;
   
   if (Sim::findObject(argv[1], actorObject) &&
       Sim::findObject(argv[4], roomObject))
   {
      roomObject->addObject(gGlobals.currentEgo);
      gGlobals.currentEgo->setPosition(Point2I(vmPtr->valueAsInt(argv[2]),
                                               vmPtr->valueAsInt(argv[3])));
   }
   
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(putActorAtObject, 3, 3, "")
{
   Actor* actorObject = nullptr;
   RoomObject* roomObject = nullptr;
   
   if (Sim::findObject(argv[1], actorObject) &&
       Sim::findObject(argv[2], roomObject))
   {
      Room* theRoom = dynamic_cast<Room*>(roomObject->getGroup());
      if (theRoom)
      {
         theRoom->addObject(gGlobals.currentEgo);
         actorObject->setPosition(roomObject->mAnchor + roomObject->mHotspot);
      }
   }
   
   return KorkApi::ConsoleValue();
}


END_SW_NS
