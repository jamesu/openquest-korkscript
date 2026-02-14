#include "engine.h"

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


TextureSlot* TextureHandle::getPtr() const
{
    return gTextureManager->resolveHandle(*this);
}


TextureManager::TextureManager()
{
   
}

TextureManager::~TextureManager()
{
   cleanup();
}

TextureHandle TextureManager::loadTexture(const std::string& path, Image* existingImage)
{
    // Already loaded?
    auto it = mPathToId.find(path);
    if (it != mPathToId.end()) {
        U32 id = it->second;
        return TextureHandle(id);
    }

    // Load new texture
    ::Texture2D tex = existingImage ? ::LoadTextureFromImage(*existingImage) : ::LoadTexture(path.c_str());
   
    if (tex.id == 0)
    {
       Con::errorf("Failed to load image '%s'", path.c_str());
       return TextureHandle();
    }

    TextureSlot* slot = new TextureSlot();
    slot->mTexture = tex;
    slot->mPath = path;

    mTextureList.allocListHandle(slot, false);

    U32 newId = TextureHandle::makeValue(slot->mAllocNumber, slot->mGeneration, true);
    mPathToId[path] = newId;
    return TextureHandle(slot);
}

void TextureManager::flushUnused()
{
   mTextureList.forEach([this](TextureSlot* slot){
      if (slot->mRefCount == 0)
      {
        mTextureList.freeListPtr(slot);
      }
   });
}

void TextureManager::cleanup() 
{
   mTextureList.forEach([this](TextureSlot* slot){
    mTextureList.freeListPtr(slot);
   });
}
