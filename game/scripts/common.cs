/* ScummC
 * Copyright (C) 2007  Alban Bedel, Gerrit Karius
 * Conversion and adaptation to KorkScript Copyright (C) 2026 James S Urquhart
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

// COMPILES
// =========================
// Resources (charsets, cursor image)
// =========================
 /*
new Charset(chset1)       { path = "vera-gui.char";   };
new Charset(chtest)       { path = "vera.char";       };
new Charset(dialogCharset){ path = "vera-small.char"; };*/


$VAR_TIMER_NEXT = 2; // run at 30 fps

// Fiber types
$SCHEDULE_SENTENCE = 0x2;

// Wait flags
$SCHEDULE_MESSAGE = 0x8;
$SCHEDULE_MESSAGE = 0x10;
$SCHEDULE_CAMERA_MOVING = 0x20;
$SCHEDULE_SENTENCE_BUSY = 0x40;

// Exception flags
$CUTSCENE_OVERRIDE = 0x1;

new ImageSet(CursorImg)   { path = "graphics/cursor/cursor.bmp"; flags = TRANSPARENT; };

// Cursor sprite shown as a room object (for hit/mask parity with SCUMMC)
new Room(ResRoom)
{
    class = BaseRoom;
    inputDelegate = ResRoom;
    realInputHandler = defaultInputHandler;

    new RoomObject(cursor)
    {
        x = 0; y = 0; w = 16; h = 16;
        description = "cursor";
        imageSet = CursorImg;
    };
};


function DisplayBase::isPerson(%this)
{
    return false;
}

function DisplayBase::isOpenable(%this)
{
    return false;
}

function DisplayBase::getPreposition(%this)
{
    return "To";
}

// Wait helpers - these replicate scumm-like waiting; 
// in this case these are ones that check object states so 
// polling is required
// waitForMessage, waitForCamera, and waitForSentence are all 
// implemented natively through flag waits.

function waitForActor(%actor)
{
    while (%actor.isMoving())
    {
        delayFiber(1);
    }
}

function Actor::waitForAnimation(%this)
{
    %lastFrame = %actor.getCurrentFrame();
    while (1)
    {
        delayFiber(1);
        %nextFrame = %actor.getCurrentFrame();
        if (%nextFrame == %lastFrame)
        {
            // Handle case where actor has been deleted
            if (!isObject(%actor))
            {
                return;
            }
        }
    }
}

function Actor::waitFor(%this)
{
    while (%actor.getState() != 0)
    {
        delayFiber(1);
    }

}

function waitForTurn(%actor)
{
    %lastDirection = %actor.getCurrentDirection();
    while (1)
    {
        delayFiber(1);
        %nextDirection = %actor.getCurrentDirection();
        if (%nextDirection == %lastDirection)
        {
            // Handle case where actor has been deleted
            if (!isObject(%actor))
            {
                return;
            }
        }
    }
}

// Common globals
$selVerb   = 0;
$altVerb   = 0;
$tryPick   = 0;

$cursorOn      = 0;
$cursorLoaded  = 0;

function ResRoom::resetMouseWatch(%this)
{
    if (isFiberRunning(%this.mouseWatchFID))
    {
        stopFiber(%this.mouseWatchFID);
    }
    %this.mouseWatchFID = ResRoom.spawnFiber(0, mouseWatch);
}

function ResRoom::stopMouseWatch(%this)
{
    if (isFiberRunning(%this.mouseWatchFID))
    {
        stopFiber(%this.mouseWatchFID);
    }
    %this.mouseWatchFID = 0;
}

function ResRoom::updateSentence(%this)
{
    $SntcLine.displayText = "";

    %str = "";
    if (isObject($sntcVerb))
    {
        %str = %str @ $sntcVerb.displayText;
    }

    if (isObject($sntcObjA))
    {
        %str = %str SPC $sntcObjA.displayText;
    }

    %str = %str SPC $sntcPrepo;

    if (isObject($sntcObjB))
    {
        %str = %str SPC $sntcObjB.displayText;
    }

    $SntcLine.displayText = %str;
}

// mouse watch script
// 
// This is responsible for setting sntcVerb, sntcObjA, sntcObjB depending on what the mouse is hovering over.
// Actual execution of commands is done in inputHandler.
function ResRoom::mouseWatch(%this)
{
    %vrb = 0; %obj = 0; %target = 0; %alt = 0;

    while (true)
    {
        if (!$cursorOn)
        {
            if ($altVerb)
            {
                $altVerb.setOn();
                $altVerb = 0;
            }
            do { breakFiber(); } while (!$cursorOn);
        }

        // NOTE: should probbaly use flag here
        if (SentenceQueue.isBusy())
        {
            echo("mouseWatch: waiting for sentence");
            breakFiber();
            continue;
        }

        // Get hover target
        %vrb = 0;
        %obj = getObjectAt($VAR_VIRT_MOUSE_X, $VAR_VIRT_MOUSE_Y);

        if (!isObject(%obj))
        {
            %obj = getActorAt($VAR_VIRT_MOUSE_X, $VAR_VIRT_MOUSE_Y);
            if (%obj == $VAR_EGO) 
            {
                %obj = 0;
            }
        }

        if (!isObject(%obj))
        {
            %obj = getVerbAt($VAR_MOUSE_X, $VAR_MOUSE_Y);
            if (%obj.inventorySlot)
            {
                %obj = $VAR_EGO.findInventory(%obj.inventorySlot + $invOffset);
                %vrb = (!$selVerb || $selVerb.getInternalName() $= PickUp) ? Verbs-->Use : $selVerb;
            }
            else 
            {
                %obj = 0;
            }
        }

        if (!isObject(%vrb)) 
        {
            %vrb = isObject($selVerb) ? $selVerb : Verbs-->WalkTo;
        }

        if ($sntcPrepo)
        {
            %target = $sntcObjB;
            if (%obj == $sntcObjA) 
            { 
                %obj = 0;
            }
        }
        else 
        {
            %target = $sntcObjA;
        }

        if (!(%vrb = $sntcVerb 
            && %obj == %target))
        {
            $sntcVerb = %vrb;

            if ($sntcPrepo)
            {
                $sntcObjB     = %obj;
            }
            else
            {
                $sntcObjA     = %obj;
            }
        }

        if (%obj)
        {
            if (%obj.isPerson())       
            {
                %alt = Verbs-->TalkTo;
            }
            else if (%obj.isOpenable())
            {
                %alt = Verbs-->Open;
            }
            else
            {
                %alt = Verbs-->LookAt;
            }
        }
        else 
        {
            %alt = 0;
        }

        if (%alt != $altVerb)
        {
            if ($altVerb) 
            { 
                $altVerb.setOn();  
            }
            if (%alt)    
            { 
                %alt.setDim();  
            }
            $altVerb = %alt;
        }

        %this.updateSentence();
        breakFiber();
    }
}

// Cursor...

function BaseRoom::showCursor(%this)
{
    if ($cursorOn) 
    {
        return;
    }

    cursorState(true);
    userPutState(true);
    $cursorOn = 1;
}

function BaseRoom::hideCursor(%this)
{
    if (!$cursorOn) 
    {
        return;
    }

    cursorState(false);
    userPutState(false);
    $cursorOn = 0;
}

// Cutscene hooks...

function BaseRoom::cutsceneStart(%type)
{
    echo("cutscene start");
    %this.hideCursor();

    if (%type > 0)
    {
        // ensure mouseWatch is stopped before hiding verbs
        breakFiber();
        Verbs.showVerbs(0);
    }
}

function BaseRoom::cutsceneEnd(%type)
{
    echo("cutscene end");
    %this.showCursor();

    if (%type > 1) 
    {
        Verbs.showVerbs(1);
    }
}

// =========================
// Sentence helpers
// =========================
function BaseRoom::resetSntc(%vrb)
{
    $sntcObjA = 0;

    if ($sntcPrepo !$= "")
    {
        $sntcPrepo = "";
        $sntcObjB = 0;
    }

    $selVerb = %vrb;
    
}

// Default verb handlers
// NOTE: all verb handlers get executed within the sentence fiber which can be cancelled at any point; 
// if you opt to run another script in a handler, be aware that unless it's appropriately flagged it will 
// continue executing even though everything else has been cancelled.

function DisplayBase::onWalkTo(%this)
{
}

function DisplayBase::onPickUp(%this, %vrb, %objA, %objB)
{
    if (%objA.isMethod(onPickup))
    {
        %returnValue = %objA.onPickup();

        if (%returnValue !$= "")
        {
            $VAR_EGO.pickupObject(%returnValue);
            %objA.setState(0);
            return;
        }
    }

    if (%objA.isPerson())
    {
        egoSay("I don't need them.");
    }
    else
    {                  
        egoSay("I don't need that.");
    }

    waitForMessage();
}

function DisplayBase::onUse(%this, %vrb, %objA, %objB)
{
    if (%objA.isPerson())
    {
        if (%objA.getId() == commanderZif.getId() || 
            %objA.getId() == ensignZob.getId())
        {
            $VAR_EGO = %objA.getId();
            cameraFollowActor($VAR_EGO);
        }
        else 
        {
            egoSay("I can't just *use* someone.");
        }
    }
    else
    {
        if (%objB)
        {
            // NOTE: in scummc version, this is implemented via a verb. We just use a named function here.
            if (%vrb.getInternalName() $= Use && 
                %objB.isMethod(%objB, onUsedWith))
            { 
                %objB.onUsedWith(objA);
            }
            else if (%objB.isPerson()) 
            {
                egoSay("I can't use that on someone!");
            }
            else
            {
                egoSay("That doesn't work.");
            }
        }
        else 
        {
            egoSay("I don't know how to operate that.");
        }
    }

    waitForMessage();
}

function DisplayBase::onGive(%this, %vrb, %objA, %objB)
{
    if (isObject(%objB) && %objB.isPerson()) 
        egoSay("I don't think i should be giving this away.");
    else                           
        egoSay("I can't do that !");
    waitForMessage();
}

function DisplayBase::onLookAt(%this, %vrb, %objA, %objB)
{
    if (%objA.isPerson()) 
        egoSay("Some kind of lifeform.");
    else                  
        egoSay("Looks pretty ordinary.");
    waitForMessage();
}

function DisplayBase::onMove(%this, %vrb, %objA, %objB)
{
    if (%objA.isPerson()) 
        egoSay("Moving them would accomplish nothing.");
    else                  
        egoSay("Moving that would accomplish nothing.");
    waitForMessage();
}

function DisplayBase::onOpen(%this, %vrb, %objA, %objB)
{
    if (%objA.isPerson()) 
    { 
        egoSay("They don't seem to open."); 
        waitForMessage();
    }
    if (!%objA.isOpenable()) 
    { 
        egoSay("That doesn't seem to open."); 
        waitForMessage();
    }

    %objA.setState(%objA, !%objA.getState());
    if (isMethod(%objA, SetBoxes))
    {
        // TOFIX startObject2(%objA, SetBoxes, [ %vrb, %objA ]);
    }
}

function DisplayBase::onSmell(%this, %vrb, %objA, %objB)
{
    if (%objA.isPerson()) 
    {
        egoSay("They have no odour.");
    }
    else
    {
        egoSay("That has no odour.");
    }
    waitForMessage();
}

function DisplayBase::onTalkTo(%this, %vrb, %objA, %objB)
{
    if (%objA.isPerson()) 
    {
        egoSay("I don't know how to communicate with them.");
    }
    else
    {
        egoSay("I don't know how to communicate with that.");
    }
    waitForMessage();
}

function DisplayBase::onDefaultAction(%this, %vrb, %objA, %objB)
{
    egoSay("Hmm. No.");
    waitForMessage();
}

// sentenceHandler uses waits -> script
function ResRoom::sentenceHandler(%verb, %objA, %objB)
{
    %owner = 0; 
    %act = 0;

    if (%verb == $SntcLine)
    {
        %verb  = $sntcVerb;
        %objA = $sntcObjA;
        %objB = $sntcObjB;
    }

    %owner = %objA.getGroup();

    // Use/Give must own first
    while (%verb.getInternalName() $= Use || 
           %verb.getInternalName() $= Give)
    {
        if (!isObject(%objB))
        {
            // NOTE: this is a script in SCUMM, instead we use a method.
            if (%objA.isMethod(getPreposition))
            {
                $sntcPrepo = %objA.getPreposition(%verb);
                if ($sntcPrepo !$= "") 
                { 
                    return; 
                }
            }
            break;
        }

        if (%owner != $VAR_EGO)
        {
            if ($tryPick == %objA) 
            { 
                $tryPick = 0; 
                return;
            }

            if (isMethod(%objA, onPickup))
            {
                $tryPick = %objA.onPickup(%objA);
            }
            else
            {
                $tryPick = %objA;
            }

            // Try alternate sentences
            SentenceQueue.push(%verb, $tryPick, %objB);
            SentenceQueue.push(Verbs-->PickUp, %objA, 0);
            return;
        }
        else 
        {
            $tryPick = 0;
        }
        break;
    }

    // Walk near room object/actor if needed
    %targetObject = 0;
    if (isObject(%objA))
    {
        %targetObject = %objA;
    }
    else if (isObject(%objB))
    {
        %targetObject = %objB;
    }

    if (isObject(%targetObject))
    {
        $VAR_EGO.walkToObject(%targetObject);
        waitForActor($VAR_EGO);

        if (%targetObject.getClassName() $= "Actor")
        {
            $VAR_EGO.faceObject(%targetObject);
        }
    }

    // Dispatch to object verb or fallback
    %handler = on @ %verb.getInternalName();
    if (%objA.isMethod(%handler))
    {
        %objA.spawnFiber(0, %handler, %verb, %objA, %objB);
        // NOTE: in this case, we're assuming "cursor off" == "busy" 
        // We would probably be better off just having a "wait" for fiber to return thing here.
        do { breakFiber();  } while (!$cursorOn);
    }
    else
    {
        %this.defaultAction(%verb, (%act ? %act : %objA), %objB);
    }

    // If verb needs objB, stop now
    if ($sntcPrepo !$= "" && !%objB) 
    {
        return;
    }

    %this.resetSntc(0);
}

// keyboardHandler uses waitForMessage/restart -> script
function BaseRoom::keyboardHandler(%this, %key)
{
    switch$ (%key)
    {
        case $KEY_O:
            egoSay("Hooo");
            break;

        case %KEY_R:
            egoSay("Let's restart."); 
            waitForMessage();
            restartGame();
            break;

        case $KEY_Q:
            shutdown();
            break;
    }
}

// Input handler; this uses the SCUMM conventions here, i.e.
// areas:
// 0 = invalid
// 1 = system ui
// 2 = room;  cmd = 0; btn = L=1/R=2/KB=0
// 3 = verbs; cmd = verb
// 4 = keyboard; cmd = keycode
// Callback is shared by all room instances, 
// unlike scumm which uses a "delegate" script we just use 
// a delegate object + handler name in ResRoom. 
// (ALSO: inputHandler can simply be overridden for room specific cases)
function BaseRoom::inputHandler(%this, %area, %cmd, %btn)
{
    echo("BASE Area=" @ %area @ " cmd=" @ %cmd @ " button=" @ %btn);
    ResRoom.inputDelegate.call(ResRoom.realInputHandler, %area, %cmd, %btn);
    ResRoom.updateSentence(); // in case its updated
}

$INVENTORY_COL = 2;
$INVENTORY_LINE = 2;
$INVENTORY_SLOTS = ($INVENTORY_COL*$INVENTORY_LINE);
$invOffset = 0;
$invOffsetMax = 0;

function BaseRoom::defaultInputHandler(%this, %area, %cmd, %btn)
{
    %invCount = 0; $invOffsetMax = 0;

    echo("Area=" @ %area @ " cmd=" @ %cmd @ " button=" @ %btn);

    if (%area == 4) 
    { 
        %this.keyboardHandler(%cmd); 
        return; 
    }

    if (%area == 3)
    {
        // NOTE: cmd is verb object so we can query metadata about it here
        if (%verb.isPreposition)
        {
            %this.resetSntc();
            return;
        }

        %cmdName = %verb.internalName;

        if (%cmdName $= invUp || 
            %cmdName $= invDown)
        {
            %invCount = getInventoryCount($VAR_EGO);
            $invOffset += ((%cmd $= invUp) ? -1 : 1) * $INVENTORY_COL;
            $invOffsetMax = ((%invCount + $INVENTORY_COL - 1) / $INVENTORY_COL - $INVENTORY_LINE) * $INVENTORY_COL;

            if ($invOffset > $invOffsetMax) $invOffset = $invOffsetMax;
            if ($invOffset < 0)             $invOffset = 0;

            echo("Inventory offset: " @ $invOffset);
            Verbs::showVerbs(1);


            %this.inventoryUpdate();
            return;
        }
    }

    // Stop current sentence and (re)start mouse watch to refresh sentence line
    SentenceQueue.cancel();
    %this.resetMouseWatch();

    %prepoSet = $sntcPrepo !$= "";

    // RMB with no selection cancels walk
    if (%btn == 2 && !(%prepoSet ? $sntcObjB : $sntcObjA))
    {
        if (isObject($VAR_EGO))
        {
            $VAR_EGO.setStanding();
        }
        %this.resetSntc(0);
        return;
    }

    // Clicked an object (room or inventory)
    if (%prepoSet ? $sntcObjB : $sntcObjA)
    {
        if (%cmd) 
        {
            $selVerb = $sntcVerb;               // keep displayed verb
        }

        if (%btn == 2 && $altVerb) 
        { 
            $selVerb = $altVerb; 
            %this.resetMouseWatch(); 
        }

        SentenceQueue.push($sntcVerb, $sntcObjA, $sntcObjB);
        return;
    }

    // Non-room area click: ignore
    if (%area != 2) return;

    // Clicked on empty room space: walk there
    if ($selVerb) 
    {
        %this.resetSntc(0);
    }

    if(isObject($VAR_EGO))
    {
        $VAR_EGO.walkTo($VAR_VIRT_MOUSE_X, $VAR_VIRT_MOUSE_Y);
    }
}

function BaseRoom::inventoryUpdate(%this)
{
    %i = 0; 
    %count = $VAR_EGO.getInventoryCount();
    echo(%count @ " obj in inv");

    for (%i = 0; %i < $INVENTORY_SLOTS; %i++)
    {
        if (%i + $invOffset < %count)
        {
            %obj = $VAR_EGO.findInventory(%i + 1 + $invOffset);
            if (isObject(%obj))
            {
                %obj.setInventoryIcon(%obj, $invSlot0 + %i);
            }
        }
        else
        {
            %verb = $invSlot0 + %i;
            %verb.setNameString("");
        }
    }
}

// =========================
// Hooks: pre-entry & quit (no waits)
// =========================
function ResRoom::preEntry(%this) 
{ 
    Actors.loadObjects(); 
}

function ResRoom::quit(%this)     
{ 
    shutdown(); 
}

function ResRoom::setupUI(%this)
{
    echo("Setup verbs:" SPC Verbs.getCount());
    foreach (%verb in Verbs)
    {
       RootUI.add(%verb);
    }
}

// =========================
// Main (engine entry) — no waits -> function
// =========================
function ResRoom::main(%this, %bootParam)
{
    // Engine hotkeys / behavior
    $VAR_MAINMENU_KEY     = 319;
    $VAR_TALKSTOP_KEY     = 46;
    $VAR_RESTART_KEY      = 322;
    $VAR_PAUSE_KEY        = " ";
    $VAR_CUTSCENEEXIT_KEY = 27;

    $VAR_GAME_VERSION = 0;
    /*$VAR_GUI_COLORS[0] = 0x00,0x00,0x43,0x00,0xD7,0x34,0x52,0x90,0x00,0x6A,
                          0x06,0x1A,0xD5,0xE5,0xE3,0xE5,0xE3,0xE5,0xE3,0xE5,
                          0xE3,0x00,0x00,0x00,0x00,0x14,0xD7,0xE5,0xE3,0xE5,
                          0xE3,0x37,0x1C,0xE5,0xE3,0xE5,0xE3,0x14,0xD7,0xE5,
                          0xE3,0xE5,0xE3,0x00,0x00,0x00,0x00,0x00,0x00,0x00;*/

    $VAR_DEBUG_PASSWORD[0] = "";

    // GUI strings
    $VAR_PAUSE_MSG[0]      = "ScummC Paused !";
    $VAR_QUIT_MSG[0]       = "Are you sure you want to quit ? (Y/N)Y";
    $VAR_RESTART_MSG[0]    = "Are you sure you want to restart ? (Y/N)Y";

    $VAR_SAVE_BTN[0]   = "Save it";
    $VAR_LOAD_BTN[0]   = "Load it";
    $VAR_PLAY_BTN[0]   = "Continue";
    $VAR_CANCEL_BTN[0] = "Cancel";
    $VAR_QUIT_BTN[0]   = "Quit";
    $VAR_OK_BTN[0]     = "Ok";

    $VAR_SAVE_MSG[0]   = "Saveing '%%s'";
    $VAR_LOAD_MSG[0]   = "Loading '%%s'";

    $VAR_MAIN_MENU_TITLE[0] = "ScummC test Menu";
    $VAR_SAVE_MENU_TITLE[0] = "Save game";
    $VAR_LOAD_MENU_TITLE[0] = "Load game";

    $VAR_NOT_SAVED_MSG[0]   = "Game NOT saved";
    $VAR_NOT_LOADED_MSG[0]  = "Game NOT loaded";

    $VAR_GAME_DISK_MSG[0]   = "Insert disk %%c";
    $VAR_ENTER_NAME_MSG[0]  = "You must enter a name";
    $VAR_SAVE_DISK_MSG[0]   = "Insert your save disk";
    $VAR_OPEN_FAILED_MSG[0] = "Failed to open %%s (%%c%%d)";
    $VAR_READ_ERROR_MSG[0]  = "Read error on disk %%c (%%c%%d)";

    // Timing & engine hook-up
    $VAR_TIMER_NEXT           = 2;

    // Preload rooms kept resident
    //loadRoom(ResRoom);
    ResRoom.lock();
    loadRoom(OfficeRoom);
    loadRoom(TitleScreen);
    loadRoom(Skyline);
    loadRoom(SecretRoom);
    OfficeRoom.lock();

    // Screen height must match room
    %this.setupUI();

    // Init default charset
    //initCharset(chtest);

    // Cursor & mouse tracking
    %this.showCursor();
    %this.resetMouseWatch();

    echo("Booting with param:" @ %bootParam);

    // Boot path
    switch (%bootParam)
    {
        case 2:
            OfficeRoom->exitToSecretRoom.setState(7);
            $OfficeRoom::didOfficeIntro = 1;
            startRoom(OfficeRoom);

        case 1:
            $OfficeRoom::didOfficeIntro = 1;
            startRoom(OfficeRoom);

        default:
            TitleScreen.setTransitionMode(2, 0, 1.0);
            startRoom(TitleScreen);
    }
}


function beginCutscene(%mode)
{
    echo("DBG: BEGIN CUTSCENE:" @ %mode);
    pushFiberSuspendFlags(0x4);
    cursorState(false);

    if(%mode > 0) 
    {
        Verbs.showVerbs(0);
        breakFiber();
        Verbs.showVerbs(0);
    }

}

function endCutscene()
{
    echo("DBG: END CUTSCENE");
    popFiberSuspendFlags();
    cursorState(true);
    Verbs.showVerbs(1);
}
