#pragma once

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


BEGIN_SW_NS


struct CostumeRenderer
{
    enum AnimFlags
    {
        LOOP=0,
        NO_LOOP=BIT(1),
        HIDE=BIT(3)
    };
   
    enum GlobalFlags
   {
       FLIP=BIT(0)
    };
   
    enum CommandOpcode : U16
    {
       CMD_IMG,    // change image
       CMD_SOUND,  // play sound
       CMD_HIDE,   // hide image
       CMD_SHOW,   // show image
       CMD_NOP,    // do nothing
       CMD_COUNT,  // movement step for walk
       CMD_FLAG,   // flag set (not used at runtime)
       CMD_END
    };
   
   static const char* opcodeMap[];

    enum
    {
        NumDirections = 4
    };
   
   enum DirectionValue : U8
   {
      NORTH,
      SOUTH,
      WEST,
      EAST
   };
   
   /*
    Layout:
    
       LimbState[numLimbs]
       For each anim:
         AnimInfo
            For each direction:
               AnimDirection
                  AnimLimbMap[]
                     LimbTrack
                        Command[]
       Frame[numFrames]
    */

    // Limb track for direction
    struct AnimDirection
    {
        U32 startLimbMap; // index into AnimLimbMap
        U32 numLimbs;     // x NumDirections
        U32 flags;        // global anim flags; loop, etc
    };

    struct AnimInfo
    {
        AnimDirection directionTracks[NumDirections]; // tracks for each dir
        StringTableEntry name;
        U32 flags; // loop, etc
    };
   
   struct LimbTrack
   {
      U32 startCmd;        // start frame of current loop
      U32 numCommands;     // 0 = invisible
      U32 flags;           // limb specific anim flags
   };

    struct AnimLimbMap
    {
       LimbTrack track;
       U32 targetLimb;   // limb the track is for
    };

    // compiled frame
    struct Frame
    {
        TextureHandle displayImage;
        Point2I displayOffset;
       U8 setFlags;
    };
   
    // compiled command
    struct Command
    {
       U16 cmd;
       U16 param;
    };

    // State of limbs
    struct LimbState
    {
        StringTableEntry name;
        LimbTrack track;
        U32 nextCmd;         // command we are on next
        U32 lastEvalFrame;   // last Frame we are displaying
    };

    // compiled state
    struct StaticState
    {
        std::vector<Frame> mFrames;
        std::vector<Command> mCommands;
        std::vector<AnimLimbMap> mLimbMap;
        std::vector<AnimInfo> mAnims;
        std::vector<StringTableEntry> mLimbNames; // base names for LimbState
        std::vector<SimWorld::Sound*> mSounds;
        U32 mFlags;
       
       void reset(bool arraysOnly=false);
    };

    // live state
    struct LiveState
    {
        U8 globalFlags;   // baked costume flags
        U8 animFlags;     // this is global ANIM flags
        S16 curAnim;      // AnimInfo we are playing
        U8 curDirection;  // current direction
        U16 curCount;     // walk counter (based on current movement direction)
        Point2F position; // display position
        Point2F delta;    // momentum
        F32 scale;
        std::vector<LimbState> mLimbState;

       void init(StaticState& state);

       void reset();
       
       void resetAnim(StaticState& state, U8 direction);

       void setAnim(StaticState& state, StringTableEntry animName, U8 direction);

       void evalCmd(StaticState& state, LimbState& limbState, Command& cmd);

       void advanceTick(StaticState& state);

       void render(StaticState& state);

       RectI getCurrentBounds(StaticState& state);

       S32 lookupAnim(StaticState& state, StringTableEntry animName);
       
       bool isAnimPlaying(StaticState& state, U32 animIdx);
    };
};


class CostumeAnim : public SimObject
{
   typedef SimObject Parent;
public:
   
   struct LimbControl
   {
      SimObjectId setId;
      U16 setCommand;
      U16 setParam;
   };
   
   struct LimbRoot
   {
      std::vector<LimbControl> entries;
      StringTableEntry limbName;
      LimbRoot* next;
      
      LimbRoot() : limbName(nullptr), next(nullptr) {;}
   };
   
   LimbRoot* mRootLookups[CostumeRenderer::NumDirections];
   U32 mAnimFlags;
   
   static bool getLimbStorage(KorkApi::Vm* vmPtr, void* obj, const KorkApi::FieldInfo* field, KorkApi::ConsoleValue arrayValue, KorkApi::TypeStorageInterface* outStorage, bool writeMode);
   
   static bool enumerateLimbStorage(void* user, KorkApi::Vm* vmPtr, KorkApi::VMObject* obj, const KorkApi::FieldInfo* field, KorkApi::FieldKeyVisitorFn visit);
   
   //  bool (*WriteDataNotifyFn)( void* obj, StringTableEntry pFieldName );
   static bool shouldWriteLimb(void* obj, StringTableEntry pFieldName);
   
   CostumeAnim();

   bool onAdd();
   
   static void initPersistFields();
   
public:
   DECLARE_CONOBJECT(CostumeAnim);
};


class Costume : public SimGroup
{
   typedef SimObject Parent;
   
public:
   Palette* mPalette;
   std::vector<StringTableEntry> mLimbNames;
   
   Point2I mBaseTalkPos;
   
   CostumeRenderer::StaticState mState;
   
   void enumerateItems(std::vector<CostumeAnim*> &anims);
   bool compileCostume();
   
   Costume();
   
   static void initPersistFields();
   
public:
   DECLARE_CONOBJECT(Costume);
};

END_SW_NS
