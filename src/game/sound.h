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
   ::Sound mSound;
   
public:
   DECLARE_CONOBJECT(Sound);
};

END_SW_NS
