#pragma once

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


class ITickable
{
   struct TickableInfo
   {
      ITickable* tickable;
      bool registered;
   };
 
public:
   virtual void onFixedTick(F32 dt) = 0;
   
   void registerTickable();
   
   void unregisterTickable();
   
   static void doFixedTick(F32 dt);
   
   static std::vector<TickableInfo> smTickList;
};
