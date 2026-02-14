#include "engine.h"

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS


DefineConsoleType(TypeLimbControlVector);
ConsoleType( limbControlVector, TypeLimbControlVector, sizeof(std::vector<CostumeAnim::LimbControl>), UINT_MAX, "" )
ConsoleTypeOpDefaultNumeric( TypeLimbControlVector )


IMPLEMENT_CONOBJECT(Charset);
IMPLEMENT_CONOBJECT(ImageSet);
IMPLEMENT_CONOBJECT(Palette);


ImageSet::ImageSet()
{
   mFormatString = StringTable->insert("");
   mFlags = FLAG_TRANSPARENT;
   mOffset = Point2I(0,0);
};

ImageSet::~ImageSet()
{
   clearImages();
}

void ImageSet::clearImages()
{
   for (Image& img : mLoadedImages)
   {
      ::UnloadImage(img);
   }
   mLoadedImages.clear();
}

void ImageSet::ensureImageLoaded(U32 n)
{
   if (mLoadedImages.size() <= n)
   {
      for (U32 i=(U32)mLoadedImages.size(); i<=n; i++)
      {
         std::string fpath = makeImageFilename(n);
         char dstName[4096];
         Con::expandPath(dstName, sizeof(dstName), fpath.c_str(), Con::getCurrentCodeBlockFullPath());
         if (Platform::isFile(dstName))
         {
            Image img = ::LoadImage(dstName);
            if (img.data)
            {
               if ((mFlags & FLAG_TRANSPARENT) != 0)
               {
                  ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
                  ImageColorReplace(&img, PINK_BG, BLANK);
               }
               mLoadedImages.push_back(img);
            }
            else
            {
               mLoadedImages.push_back(Image());
            }
         }
         else
         {
            mLoadedImages.push_back(Image());
         }
      }
   }
}

std::string ImageSet::makeImageFilename(U32 n)
{
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << n;

    std::string path = mFormatString;
    size_t pos = path.find("??");
    if (pos != std::string::npos) {
        path.replace(pos, 2, ss.str());
    }
    return path;
}

void ImageSet::initPersistFields()
{
   Parent::initPersistFields();
   
   addField("format", TypeString, Offset(mFormatString, ImageSet));
   addField("offset", TypePoint2I, Offset(mOffset, ImageSet));
   addField("flags", TypeS32, Offset(mFlags, ImageSet));
}
                    

ConsoleMethodValue(ImageSet, pick, 3, 4, "(start, end)")
{
  S32 startValue = vmPtr->valueAsInt(argv[2]);
  S32 endValue = argc < 4 ? startValue : vmPtr->valueAsInt(argv[3]);
  
  if (!(endValue >= startValue && startValue >= 0 && endValue >= 0)
      && endValue < object->mLoadedImages.size())
  {
     return KorkApi::ConsoleValue();
  }
  
  std::vector<CostumeAnim::LimbControl> limbs;
  for (U32 i=startValue; i<=endValue; i++)
  {
     CostumeAnim::LimbControl limb = {};
     limb.setId = object->getId();
     limb.setCommand = CostumeRenderer::CMD_IMG;
     limb.setParam = i;
     limbs.push_back(limb);
  }
  
  // -> output as value
  KorkApi::ConsoleValue retValue = vmPtr->getTypeReturn(TypeLimbControlVector);
  
  KorkApi::TypeStorageInterface inputStorage = {};
  KorkApi::TypeStorageInterface outputStorage = {};
  
  vmPtr->initFixedTypeStorage(&limbs, TypeLimbControlVector, true, &inputStorage);
  vmPtr->initReturnTypeStorage(sizeof(CostumeAnim::LimbControl) * limbs.size() + sizeof(U32), TypeLimbControlVector, &outputStorage);
  vmPtr->castValue(TypeLimbControlVector, &inputStorage, &outputStorage, NULL, 0);
  return outputStorage.data.storageAddress;
}


ConsoleGetType( TypeLimbControlVector )
{
   std::vector<CostumeAnim::LimbControl>* vec = nullptr;
   static std::vector<CostumeAnim::LimbControl> workVec;

   auto parseLimb = +[](const char* word, CostumeAnim::LimbControl& outLimb){
      const char* paramName = StringUnit::getUnit(word, 0, ":");
      U32 unitCount = StringUnit::getUnitCount(word, ":");
      
      if (strcasecmp(paramName, "IMG") == 0)
      {
         // object:frame
         const char* objName = StringUnit::getUnit(word, 1, ":");
         ImageSet* imgSet = nullptr;
         if (!Sim::findObject(objName, imgSet))
         {
            return false;
         }

         outLimb.setId = imgSet->getId();
         outLimb.setCommand = CostumeRenderer::CMD_IMG;
         outLimb.setParam = (U16)atoi(StringUnit::getUnit(word, 2, ":"));
         return true;
      }
      else if (strcasecmp(paramName, "SOUND") == 0)
      {
         const char* objName = StringUnit::getUnit(word, 1, ":");
         SimWorld::Sound* sound = nullptr;
         if (!Sim::findObject(objName, sound))
         {
            return false;
         }
         
         outLimb.setId = sound->getId();
         outLimb.setCommand = CostumeRenderer::CMD_SOUND;
         outLimb.setParam = 0;
         return true;
      }
      else
      {
         // Basic commands
         for (U32 i=CostumeRenderer::CMD_HIDE; i<CostumeRenderer::CMD_END; i++)
         {
            if (strcasecmp(paramName, CostumeRenderer::opcodeMap[i]) == 0)
            {
               outLimb.setId = 0;
               outLimb.setCommand = i;
               outLimb.setParam = atoi(StringUnit::getUnit(word, 1, ":"));
               return true;
            }
         }
      }
      return false;
   };


   if (!inputStorage->isField)
   {
      if (!outputStorage->isField &&
          (requestedType == TypeLimbControlVector) &&
          inputStorage->data.argc == 1 &&
          inputStorage->data.storageRegister->typeId == TypeLimbControlVector)
      {
         // Just copy data
         U32* ptr = (U32*)ConsoleGetInputStoragePtr();
         U32 numElements = ptr ? *ptr++ : 0;
         U32 dataSize = (numElements * sizeof(CostumeAnim::LimbControl));
         outputStorage->FinalizeStorage(outputStorage, sizeof(U32) +  dataSize);
         U32* returnBuffer = (U32*)ConsoleGetOutputStoragePtr();
         *returnBuffer++ = numElements;
         
         if (ptr)
         {
            memcpy(returnBuffer, ptr, dataSize);
         }
         
         if (outputStorage->data.storageRegister)
         {
            *outputStorage->data.storageRegister = outputStorage->data.storageAddress;
         }
      
         return true;
      }
      else
      {
         // Need to take long path
         vec = &workVec;
         vec->clear();
         
         if (inputStorage->data.argc > 0)
         {
            for (U32 i=0; i<inputStorage->data.argc; i++)
            {
               ConsoleValue val = inputStorage->data.storageRegister[i];

               if (val.typeId == TypeLimbControlVector)
               {
                  U32* storagePtr = (U32*)val.evaluatePtr(vmPtr->getAllocBase());
                  U32 numElems = storagePtr[0];
                  CostumeAnim::LimbControl* data = (CostumeAnim::LimbControl*)(storagePtr+1);
                  
                  for (S32 i=0; i<numElems; i++)
                  {
                     vec->push_back(data[i]);
                  }
               }
               else
               {
                  const char* values = vmPtr->valueAsString(inputStorage->data.storageRegister[i]);
                  if (!values) values = "";

                  U32 numValues = StringUnit::getUnitCount(values, " ");
                  char buffer[128];

                  for (U32 i=0; i<numValues; i++)
                  {
                     CostumeAnim::LimbControl outLimb;
                     const char* word = StringUnit::getUnit(values, i, " ", buffer, 128);
                     if (parseLimb(word, outLimb))
                     {
                        vec->push_back(outLimb);
                     }
                  }
               }
            }
         }
      }
   }
   else
   {
      vec = (std::vector<CostumeAnim::LimbControl>*)ConsoleGetInputStoragePtr();
   }

   // Convert native vector to the requested output.

   if (outputStorage->isField && requestedType == TypeLimbControlVector)
   {
      auto* outputVec = (std::vector<CostumeAnim::LimbControl>*)ConsoleGetOutputStoragePtr();
      *outputVec = *vec;
      return true;
   }
   else if (requestedType == TypeLimbControlVector)
   {
      // Need to convert back to serialized variant
      outputStorage->FinalizeStorage(outputStorage, (vec->size() * sizeof(S32)) + sizeof(CostumeAnim::LimbControl));
      U32* vecCount = (U32*)ConsoleGetOutputStoragePtr();
      *vecCount++ = vec->size();
      
      std::copy(vec->begin(), vec->end(), (CostumeAnim::LimbControl*)vecCount);
      
      if (outputStorage->data.storageRegister)
      {
         *outputStorage->data.storageRegister = outputStorage->data.storageAddress;
      }
      
      return true;
   }
   else if (requestedType != KorkApi::ConsoleValue::TypeInternalString)
   {
      // just treat as empty for now
      outputStorage->data.argc = 1;
      outputStorage->data.storageAddress = KorkApi::ConsoleValue();
      *outputStorage->data.storageRegister = outputStorage->data.storageAddress;
      return true;
   }
   else
   {
      // Serialize to a space-separated string (safe conservative buffer, then finalize).
      S32 maxReturn = 2048;
      outputStorage->ResizeStorage(outputStorage, maxReturn);
      char* returnBuffer = (char*)outputStorage->data.storageAddress.evaluatePtr(vmPtr->getAllocBase());
      returnBuffer[0] = '\0';
      S32 returnLen = 0;
      
      for (U32 i = 0; i < (U32)vec->size(); i++)
      {
         CostumeAnim::LimbControl* control = &(*vec)[i];
         if (control->setCommand >= CostumeRenderer::CMD_END)
            continue;
         
         if (control->setId >= 0)
         {
            snprintf(returnBuffer + returnLen, maxReturn - returnLen,
                     "%s:%i", i == 0 ? "" : " ",
                     CostumeRenderer::opcodeMap[control->setCommand],
                     control->setId);
         }
         else
         {
            snprintf(returnBuffer + returnLen, maxReturn - returnLen,
                     "%s:%u", i == 0 ? "" : " ",
                     CostumeRenderer::opcodeMap[control->setCommand],
                     control->setParam);
         }
         
         returnLen = strlen(returnBuffer);
      }

      outputStorage->FinalizeStorage(outputStorage, returnLen + 1);

      if (outputStorage->data.storageRegister)
         *outputStorage->data.storageRegister = outputStorage->data.storageAddress;

      return true;
   }
   
   return false;
}


END_SW_NS
