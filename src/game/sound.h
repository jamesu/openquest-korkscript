#pragma once

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS


class Sound : public SimObject
{
   typedef SimObject Parent;
public:
   
   StringTableEntry mPath;
   U32 mChannel;
   ::Sound mSound;

   Sound();

   bool onAdd();
   void onRemove();

   void play();

   void updateResources();

   static void initPersistFields();
   
public:
   DECLARE_CONOBJECT(Sound);
};

END_SW_NS
