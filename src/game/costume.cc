#include "engine.h"

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS


IMPLEMENT_CONOBJECT(CostumeAnim);
IMPLEMENT_CONOBJECT(Costume);

const char* CostumeRenderer::opcodeMap[] = {
   "IMG",
   "SOUND",
   "HIDE",
   "SHOW",
   "NOP",
   "COUNT",
   "FLAG",
   ""
};


bool CostumeAnim::getLimbStorage(KorkApi::Vm* vmPtr, void* obj, const KorkApi::FieldInfo* field, KorkApi::ConsoleValue arrayValue, KorkApi::TypeStorageInterface* outStorage, bool writeMode)
{
   StringTableEntry steArray = StringTable->insert(vmPtr->valueAsString(arrayValue));
   
   LimbRoot* foundEntry = nullptr;
   LimbRoot** entryRoot = (LimbRoot**)(((uintptr_t)obj) + field->offset);
   for (LimbRoot* ctrl = *entryRoot; ctrl; ctrl = ctrl->next)
   {
      if (ctrl->limbName == steArray)
      {
         foundEntry = ctrl;
         break;
      }
   }
   
   // Alloc new entry
   if (foundEntry == NULL)
   {
      if (!writeMode)
      {
         return false;
      }
      
      foundEntry = new LimbRoot();
      foundEntry->next = *entryRoot;
      *entryRoot = foundEntry;
      foundEntry->limbName = steArray;
   }
   
   *outStorage = {};
   return vmPtr->initFixedTypeStorage(&foundEntry->entries, TypeLimbControlVector, true, outStorage);
}

bool CostumeAnim::enumerateLimbStorage(void* user, KorkApi::Vm* vmPtr, KorkApi::VMObject* obj, const KorkApi::FieldInfo* field, KorkApi::FieldKeyVisitorFn visit)
{
   // TODO: visit all root entries and enumerate
   // typedef bool (*FieldKeyVisitorFn)( void* user, KorkApi::Vm* vmPtr, KorkApi::VMObject* obj, KorkApi::ConsoleValue key, KorkApi::ConsoleValue value );
   //visit(field->fieldUserPtr, vmPtr, obj, KorkApi::ConsoleValue::makeString("N"), );
   return false;
}

//  bool (*WriteDataNotifyFn)( void* obj, StringTableEntry pFieldName );
bool CostumeAnim::shouldWriteLimb(void* obj, StringTableEntry pFieldName)
{
   // TODO: should check if pFieldName entry has entries
   return true;
}

CostumeAnim::CostumeAnim()
{
   memset(mRootLookups, 0, sizeof(mRootLookups));
   mAnimFlags = 0;
}

bool CostumeAnim::onAdd()
{
   if (Parent::onAdd())
   {
      return true;
   }
   return false;
}

void CostumeAnim::initPersistFields()
{
   Parent::initPersistFields();
   
   addProtectedField("N", TypeLimbControlVector, Offset(mRootLookups[0], CostumeAnim), nullptr, getLimbStorage, enumerateLimbStorage, shouldWriteLimb);
   addProtectedField("S", TypeLimbControlVector, Offset(mRootLookups[1], CostumeAnim), nullptr, getLimbStorage, enumerateLimbStorage, shouldWriteLimb);
   addProtectedField("E", TypeLimbControlVector, Offset(mRootLookups[2], CostumeAnim), nullptr, getLimbStorage, enumerateLimbStorage, shouldWriteLimb);
   addProtectedField("W", TypeLimbControlVector, Offset(mRootLookups[3], CostumeAnim), nullptr, getLimbStorage, enumerateLimbStorage, shouldWriteLimb);
   
   addField("flags", TypeS32, Offset(mAnimFlags, CostumeAnim));
}

Costume::Costume()
{
   mPalette = NULL;
   mState.reset();
}

void Costume::initPersistFields()
{
   Parent::initPersistFields();
   
   addField("limbs", TypeStringTableEntryVector, Offset(mLimbNames, Costume), "");
   addField("flags", TypeS32, Offset(mState.mFlags, Costume), "");
   addField("palette", TypeSimObjectPtr, Offset(mPalette, Costume), "");
}


ConsoleMethodValue(Costume, compileCostume, 2, 2, "")
{
   return KorkApi::ConsoleValue::makeUnsigned(object->compileCostume());
}


bool Costume::compileCostume()
{
   mState.reset(true);
   
   Con::printf("Costume %s...", getName());
   
   if (strcasecmp(getName(), "ZifCostume") == 0)
   {
      Con::printf("ziffy");
   }
   
   mState.mLimbNames = mLimbNames;
   
   // Assemble limb names
   
   // Print out plan
   for (SimObject* obj : objectList)
   {
      CostumeAnim* anim = dynamic_cast<CostumeAnim*>(obj);
      if (anim)
      {
         Con::printf("-- Anim %s --", anim->getInternalName());
         
         CostumeRenderer::AnimInfo animInfo = {};
         animInfo.name = StringTable->insert(anim->getInternalName());
         
         for (U32 i=0; i<CostumeRenderer::NumDirections; i++)
         {
            CostumeRenderer::AnimDirection animDir = {};
            animDir.startLimbMap = (U32)mState.mLimbMap.size();
            animDir.flags = anim->mAnimFlags;
            
            for (CostumeAnim::LimbRoot* rootLimb = anim->mRootLookups[i];
                 rootLimb;
                 rootLimb = rootLimb->next)
            {
               Con::printf("Limb %s DIR: %i\n", rootLimb->limbName, i);
               
               auto itr = std::find(mLimbNames.begin(), mLimbNames.end(), rootLimb->limbName);
               if (itr == mLimbNames.end())
               {
                  Con::printf("Skipping (not in costume)");
                  continue;
               }
               
               U32 localIndex = (U32)(itr - mLimbNames.begin());
               
               CostumeRenderer::AnimLimbMap limbRoot = {};
               limbRoot.targetLimb = localIndex;
               limbRoot.track.startCmd = mState.mCommands.size();
               animDir.numLimbs++;
               
               // NOTE: flags are merged into the limb track
               
               CostumeRenderer::Frame buildFrame = {};
               
               for (CostumeAnim::LimbControl& ctrl : rootLimb->entries)
               {
                  CostumeRenderer::Command cmd = {};
                  cmd.cmd = ctrl.setCommand;
                  
                  if (ctrl.setCommand == CostumeRenderer::CMD_IMG)
                  {
                     // Find image in list
                     
                     ImageSet* theSet = NULL;
                     if (!Sim::findObject(ctrl.setId, theSet))
                     {
                        Con::errorf("Cant find ImageSet %i", ctrl.setId);
                        mState.reset();
                        return false;
                     }
                     
                     // set frame number
                     CostumeRenderer::Frame frame = {};
                     theSet->ensureImageLoaded(ctrl.setParam);
                     frame.displayImage = gTextureManager->loadTexture(theSet->makeImageFilename(ctrl.setParam),
                                                                       &theSet->mLoadedImages[ctrl.setParam]);
                     frame.displayOffset = theSet->mOffset;
                     frame.setFlags = (U8)theSet->mFlags;
                     cmd.param = mState.mFrames.size();
                     mState.mFrames.push_back(frame);
                     
                     if (frame.displayImage.getNum() == 0)
                     {
                        Con::errorf("Cant load image %i from set %s [%s]", ctrl.setParam, theSet->getInternalName(), theSet->makeImageFilename(ctrl.setParam).c_str());
                     }
                  }
                  else if (ctrl.setCommand == CostumeRenderer::CMD_SOUND)
                  {
                     // Find sound in list
                     
                     SimWorld::Sound* theSound = NULL;
                     if (!Sim::findObject(ctrl.setId, theSound))
                     {
                        Con::errorf("Cant find Sound %i", ctrl.setId);
                        mState.reset();
                        return false;
                     }
                     
                     auto itr = std::find(mState.mSounds.begin(), mState.mSounds.end(), theSound);
                     if (itr == mState.mSounds.end())
                     {
                        cmd.param = mState.mSounds.size();
                        mState.mSounds.push_back(theSound);
                     }
                     else
                     {
                        cmd.param = (U32)(itr - mState.mSounds.begin());
                     }
                  }
                  else if (ctrl.setCommand == CostumeRenderer::CMD_FLAG)
                  {
                     limbRoot.track.flags |= ctrl.setParam;
                     continue;
                  }
                  else
                  {
                     cmd.param = ctrl.setParam;
                  }
                  
                  
                  limbRoot.track.numCommands++;
                  mState.mCommands.push_back(cmd);
               }
               
               mState.mLimbMap.push_back(limbRoot);
            }
            
            animInfo.directionTracks[i] = animDir;
         }
         
         mState.mAnims.push_back(animInfo);
      }
   }
   
   Con::printf("Compile complete");
}


void CostumeRenderer::StaticState::reset(bool arraysOnly)
{
   mFrames.clear();
   mCommands.clear();
   mLimbMap.clear();
   mAnims.clear();
   mLimbNames.clear();
   if (!arraysOnly)
   {
      mFlags = 0;
   }
}

void CostumeRenderer::LiveState::init(StaticState& state)
{
   reset();
   mLimbState.resize(state.mLimbNames.size());
   for (U32 i=0; i<mLimbState.size(); i++)
   {
      mLimbState[i].name = state.mLimbNames[i];
      mLimbState[i].track = {};
      mLimbState[i].nextCmd = 0;
      mLimbState[i].lastEvalFrame = 0;
   }
   
   globalFlags = (U8)state.mFlags;
}

void CostumeRenderer::LiveState::reset()
{
   globalFlags = 0;
   animFlags = 0;
   curAnim = 0;
   curTalkAnim = -1;
   curDirection = 0;
   position = Point2F(0.0f, 0.0f);
   mLimbState.clear();
   position = Point2F(0.0f, 0.0f);
   delta = Point2F(0.0f, 0.0f);
   scale = 1.0f;
}

void CostumeRenderer::LiveState::resetAnim(StaticState& state, U8 direction, bool talking)
{
   S16 animNumber = talking ? curTalkAnim : curAnim;
   if (animNumber < 0)
   {
      return;
   }
   
   AnimInfo& animInfo = state.mAnims[animNumber];
   AnimDirection& dirInfo = animInfo.directionTracks[direction];
   
   for (U32 k=0; k<dirInfo.numLimbs; k++)
   {
      AnimLimbMap& limbTrack = state.mLimbMap[dirInfo.startLimbMap + k];
      LimbState& liveLimb = mLimbState[limbTrack.targetLimb];
      liveLimb.track = limbTrack.track;
      liveLimb.nextCmd = 0;
   }
}

void CostumeRenderer::LiveState::setAnim(StaticState& state, StringTableEntry animName, U8 direction, bool talking)
{
   for (U32 i=0; i<state.mAnims.size(); i++)
   {
      AnimInfo& animInfo = state.mAnims[i];
      animFlags = animInfo.flags;
      
      if (animInfo.name == animName)
      {
         // Found it, reset to this anim
         AnimDirection& dirInfo = animInfo.directionTracks[direction];
         
         for (U32 k=0; k<dirInfo.numLimbs; k++)
         {
            AnimLimbMap& limbTrack = state.mLimbMap[dirInfo.startLimbMap + k];
            LimbState& liveLimb = mLimbState[limbTrack.targetLimb];
            liveLimb.track = limbTrack.track;
            liveLimb.nextCmd = 0;
         }
         
         // Track current anim
         if (talking)
         {
            curTalkAnim = i;
         }
         else
         {
            curAnim = i;
         }
         
         return;
      }
   }
}

void CostumeRenderer::LiveState::evalCmd(StaticState& state, LimbState& limbState, Command& cmd)
{
   switch (cmd.cmd)
   {
      case CMD_IMG:
         limbState.lastEvalFrame = cmd.param;
         break;
      case CMD_HIDE:
         limbState.track.flags |= HIDE;
         break;
      case CMD_SHOW:
         limbState.track.flags &= ~HIDE;
         break;
      case CMD_COUNT:
         curCount++;
         break;
      case CMD_SOUND:
         // TODO
         break;
      default:
         break;
   }
   
   if (limbState.nextCmd+1 < limbState.track.numCommands)
   {
      limbState.nextCmd++;
   }
   else if ((limbState.track.flags & NO_LOOP) == 0)
   {
      // loop back to start
      limbState.nextCmd = 0;
   }
}

void CostumeRenderer::LiveState::advanceTick(StaticState& state)
{
   for (LimbState& limbState : mLimbState)
   {
      if (limbState.nextCmd < limbState.track.numCommands)
      {
         evalCmd(state, limbState, state.mCommands[limbState.track.startCmd + limbState.nextCmd]);
      }
   }
}


void CostumeRenderer::LiveState::render(CostumeRenderer::StaticState& state)
{
   bool doFlip = false;
   
   if ((globalFlags & CostumeRenderer::FLIP) != 0)
   {
      // Ok, we need to flip if the current direction is west
      if (curDirection == CostumeRenderer::WEST)
      {
         doFlip = true;
      }
   }
   
   for (LimbState& limbState : mLimbState)
   {
      if (limbState.lastEvalFrame < state.mFrames.size() &&
          (limbState.track.flags & HIDE) == 0)
      {
         // Grab frame and draw
         Frame& frame = state.mFrames[limbState.lastEvalFrame];
         Point2F drawPos = position;
         
         TextureSlot* slot = gTextureManager->resolveHandle(frame.displayImage);
         if (slot)
         {
            if (doFlip)
            {
               drawPos += (Point2F(-(frame.displayOffset.x + slot->mTexture.width), frame.displayOffset.y)) * scale;
            }
            else
            {
               drawPos += (Point2F(frame.displayOffset.x, frame.displayOffset.y)) * scale;
            }
            
            ::Rectangle source = {0, 0, (float)slot->mTexture.width, (float)slot->mTexture.height};
            ::Rectangle dest = {drawPos.x, drawPos.y, slot->mTexture.width * scale, slot->mTexture.height * scale};
            Vector2 origin = {};
            
            if (doFlip)
            {
               source.x = (float)slot->mTexture.width;
               source.width = -(float)slot->mTexture.width;
            }
            
            if ((frame.setFlags & SimWorld::ImageSet::FLAG_TRANSPARENT) != 0)
            {
               BeginBlendMode(BLEND_ALPHA);
            }
            
            DrawTexturePro(slot->mTexture, source, dest, origin, 0.0f, WHITE);
            
            if ((frame.setFlags & SimWorld::ImageSet::FLAG_TRANSPARENT) != 0)
            {
               EndBlendMode();
            }
         }
      }
   }
}

END_SW_NS
