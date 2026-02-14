#pragma once

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


class TextureHandle;


class TextureSlot
{
    friend class TextureHandle;
    friend class TextureManager;
    friend class FreeListPtr<TextureSlot, TextureHandle, std::vector>;
    friend class FreeListHandle::Basic32;

public:
   U32 mAllocNumber;
   U8 mGeneration : 7;
   S32 mRefCount;

   ::Texture2D mTexture;
   std::string mPath;

   TextureSlot() : mAllocNumber(0), mGeneration(0), mTexture({}), mRefCount(0) {;}


    void incRef()
    {
        mRefCount++;
    }

    void decRef()
    {
        if (mRefCount > 0) mRefCount--;
    }

    void reset()
    {
        if (mTexture.id != 0)
        {
            ::UnloadTexture(mTexture);
        }
        mPath = "";
    }

};

class TextureHandle : public FreeListHandle::Basic32
{
public:
   TextureHandle() { parts.heavyRef = (U32)1; }
   TextureHandle(std::nullptr_t) { parts.heavyRef = (U32)1; }
   explicit TextureHandle(U32 fullAllocNumber, bool isHeavy=true)
   {
      value = fullAllocNumber;
      parts.heavyRef = (U32)isHeavy;
      if (isHeavy)
      {
         TextureSlot* obj = getPtr();
         if (obj)
            obj->incRef();
      }
   }
   explicit TextureHandle(U32 num, U8 gen, bool isHeavy=true) : Basic32(num, gen, isHeavy)
   {
      if (isHeavy)
      {
         TextureSlot* obj = getPtr();
         if (obj)
            obj->incRef();
      }
   }
   TextureHandle(const TextureHandle& other)
   {
      TextureSlot* newTO = other.getPtr();
      if (newTO && other.isHeavyRef())
      {
         newTO->incRef();
      }
      value = other.value;
   }
   TextureHandle(TextureSlot* slot, bool isHeavy=true) : FreeListHandle::Basic32(0, isHeavy)
   {
    setSlot(slot);
   }
   ~TextureHandle()
   {
      if (parts.index != 0)
      {
         setSlot(nullptr);
      }
   }

   void setSlot(TextureSlot* newSlot)
   {
      if (isHeavyRef())
      {
         TextureSlot* existSlot = getPtr();
         if (existSlot != newSlot)
         {
            if (newSlot != nullptr)
               newSlot->incRef();
            if (existSlot != nullptr)
               existSlot->decRef();
         }
      }
      value = newSlot != nullptr ? TextureHandle::makeValue(newSlot->mAllocNumber, newSlot->mGeneration, isHeavyRef()) :
                                TextureHandle::makeValue(0,0, isHeavyRef());
   }
   
   inline TextureHandle& operator=(std::nullptr_t) {
      setSlot(nullptr);
      return *this;
   }
   
   inline TextureHandle& operator=(const TextureHandle& other) {
      setSlot(other.getPtr());
      return *this;
   }
   
   inline bool operator==(const TextureHandle& other) const {
      return value == other.value;
   }
   inline bool operator!=(const TextureHandle& other) const {
      return value != other.value;
   }
   

   TextureSlot* getPtr() const;
};

class TextureManager 
{
public:
    FreeListPtr<TextureSlot, TextureHandle, std::vector> mTextureList;

   TextureManager();
   ~TextureManager();

    TextureHandle loadTexture(const std::string& path, Image* existingImage = NULL);


    inline TextureSlot* resolveHandle(const TextureHandle& h);

    void flushUnused();

    void cleanup() ;

private:

    std::unordered_map<std::string, U32> mPathToId; // path -> slot handle
    std::vector<TextureSlot*> mSlots;                             // id-1 indexing
};

inline TextureSlot* TextureManager::resolveHandle(const TextureHandle& h)
{
    return mTextureList.getItem(h.getValue());
}
