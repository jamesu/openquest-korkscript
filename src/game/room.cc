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


bool Room::onAdd()
{
   if (Parent::onAdd())
   {
      registerTickable();
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
   mPosition = newPosition;
   mExtent = newExtent;
   mRenderState.screenSize.x = mExtent.x;
   mRenderState.screenSize.y = mExtent.y;
}

void Room::updateResources()
{
   if (mBoxFileName && mBoxFileName[0] != '\0')
   {
      mBoxes.reset();
      FileStream fs;
      if (fs.open(mBoxFileName, FileStream::Read))
      {
         mBoxes.read(fs);
      }
   }
   
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
   
   DrawRectangle(offset.x, offset.y, drawRect.extent.x, drawRect.extent.y, (Color){0,0,0,255});
   
   // Transform
   RectI globalRect = WorldRectToScreen(mRenderState.clipRect, globalCam);
   BeginScissorMode(globalRect.point.x, globalRect.point.y, globalRect.extent.x, globalRect.extent.y);
   
   TextureSlot* slot = gTextureManager->resolveHandle(mRenderState.backgroundImage);
   if (slot)
   {
      Rectangle dest = { (float)offset.x, (float)offset.y, (float)slot->mTexture.width, (float)slot->mTexture.height };
      DrawTexturePro(slot->mTexture, source, dest, origin, 0.0f, WHITE);
   }
   
   Point2F scalingFactor = Point2F(mRenderState.screenSize.x / 320.0f, mRenderState.screenSize.y / 200.0f);
   
   for (BoxInfo::Box& box : mBoxes.boxes)
   {
      if (box.numPoints >= 4)
      {
         Point2I* points = mBoxes.points.data() + box.startPoint;
         
         Vector2 prevPoint = (Vector2){points[0].x * scalingFactor.x, points[0].y * scalingFactor.y};
         Vector2 originPoint = prevPoint;
         for (U32 i=1; i<box.numPoints; i++)
         {
            Vector2 curPoint = (Vector2){points[i].x * scalingFactor.x, points[i].y * scalingFactor.y};
            ::DrawLineV(prevPoint, curPoint, (Color){255,255,255,255});
            prevPoint = curPoint;
         }
         
         ::DrawLineV(prevPoint, originPoint, (Color){255,255,255,255});
      }
   }
   
   
   for (SimObject* obj : objectList)
   {
      Actor* actor = dynamic_cast<Actor*>(obj);
      if (actor)
      {
         Point2I childPosition = offset + actor->getPosition();
         RectI childClip(childPosition, actor->getExtent());
         actor->onRender(childPosition, childClip, globalCam);
      }
   }
   
   
   EndScissorMode();
   
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
   addField("zPlanes", TypeString, Offset(mImageFileName, Room), RoomRender::NumZPlanes);
}

ConsoleMethodValue(Room, setTransitionMode, 5, 5, "mode, param, time")
{
   object->setTransitionMode(vmPtr->valueAsInt(argv[2]), vmPtr->valueAsInt(argv[3]), vmPtr->valueAsFloat(argv[4]));
   return KorkApi::ConsoleValue();
}


END_SW_NS
