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
new Charset(chset1)       { path = "vera-gui.char";   };
new Charset(chtest)       { path = "vera.char";       };
new Charset(dialogCharset){ path = "vera-small.char"; };

new ImageSet(CursorImg)   { path = "graphics/cursor/cursor.bmp"; flags = TRANSPARENT; };

// Cursor sprite shown as a room object (for hit/mask parity with SCUMMC)
new Room(ResRoom)
{
    new RoomObject(cursor)
    {
        x = 0; y = 0; w = 16; h = 16;
        description = "cursor";
        imageSet = CursorImg;
    };
};

// =========================
// Globals (mirror SCUMMC vars used here)
// =========================
$selVerb   = 0;
$altVerb   = 0;
$tryPick   = 0;

$cursorOn      = 0;
$cursorLoaded  = 0;

// =========================
// Local test (no waits)
// =========================
function ResRoom::localTest(%this) { echo("Ltst"); }

// =========================
// Mouse watch (runs continuously; uses breakScript) -> script
// =========================
function ResRoom::mouseWatch(%this)
{
    %vrb = 0; %obj = 0; %target = 0; %alt = 0; %desc = 0;

    while (true)
    {
        if (!$cursorOn)
        {
            if ($altVerb)
            {
                setCurrentVerb($altVerb); setVerbOn(); redrawVerb();
                $altVerb = 0;
            }
            do { breakScript(); } while (!$cursorOn);
        }

        if (isScriptRunning($VAR_SENTENCE_SCRIPT))
        {
            breakScript();
            continue;
        }

        // ---- Read current state / hover target ----
        %vrb = 0; %desc = 0;

        %obj = getObjectAt($VAR_VIRT_MOUSE_X, $VAR_VIRT_MOUSE_Y);

        if (!isObject(%obj))
        {
            %obj = getActorAt($VAR_VIRT_MOUSE_X, $VAR_VIRT_MOUSE_Y);
            if (%obj == $VAR_EGO) %obj = 0;
            else if ($actorObject[%obj]) %desc = $actorObject[%obj];
        }

        if (!isObject(%obj))
        {
            %obj = getVerbAt($VAR_MOUSE_X, $VAR_MOUSE_Y);
            if (%obj >= $invSlot0 && %obj <= invSlot7)
            {
                %obj = findInventory($VAR_EGO, %obj - $invSlot0 + 1 + $invOffset);
                %vrb = (!$selVerb || $selVerb == PickUp) ? Use : $selVerb;
            }
            else %obj = 0;
        }

        if (%vrb $= "") %vrb = ($selVerb ? $selVerb : WalkTo);

        // Description object defaults to %obj
        if (%desc $= "") %desc = %obj;

        if ($sntcPrepo)
        {
            %target = $sntcObjB;
            if (%obj == sntcObjA) { %obj = 0; %desc = 0; }
        }
        else 
        {
            %target = $sntcObjA;
        }

        if (%vrb == sntcVerb && %obj == %target)
        {
            $sntcVerb = %vrb;

            if ($sntcPrepo)
            {
                $sntcObjB     = %obj;
                $sntcObjBDesc = %desc;
            }
            else
            {
                $sntcObjA     = %obj;
                $sntcObjADesc = %desc;
            }

            setCurrentVerb($SntcLine);
            redrawVerb();
        }

        // ---- Alt verb hinting ----
        if (%obj)
        {
            // TOFIX
            //if (%obj is Person)       %alt = TalkTo;
            //else if (%obj is Openable) %alt = Open;
            //else                       %alt = LookAt;
        }
        else 
        {
            %alt = 0;
        }

        if (%alt != $altVerb)
        {
            if ($altVerb) { setCurrentVerb($altVerb); setVerbOn(); redrawVerb(); }
            if (%alt)    { setCurrentVerb(%alt); verbDim(); redrawVerb(); }
            $altVerb = %alt;
        }

        breakScript();
    }
}

// =========================
// Cursor show/hide (no waits)
// =========================
function ResRoom::showCursor(%this)
{
    if ($cursorOn) return;
/*
    unless (cursorLoaded)
    {
        cursorLoaded = 1;
        loadFlObject(cursor, ResRoom);
        setCursorImage(cursor, ResRoom);
        setCursorTransparency(31);
    }
*/
    cursorOn();
    userPutOn();
    $cursorOn = 1;
}

function ResRoom::hideCursor(%this)
{
    if (!$cursorOn) return;
    cursorOff();
    userPutOff();
    $cursorOn = 0;
}

// =========================
// Cutscene hooks (no waits)
// =========================
function ResRoom::cutsceneStart(%type)
{
    echo("cutscene start");
    %this.hideCursor();
    if (%type > 0)
    {
        // ensure mouseWatch is stopped before hiding verbs
        breakScript();
        Verbs.showVerbs(0);
    }
}

function ResRoom::cutsceneEnd(%type)
{
    echo("cutscene end");
    %this.showCursor();
    if (%type > 1) Verbs.showVerbs(1);
}

// =========================
// Sentence helpers
// =========================
function ResRoom::resetSntc(%vrb)
{
    $sntcObjA = 0;
    $sntcObjADesc = 0;

    if ($sntcPrepo)
    {
        $sntcPrepo = "";
        $sntcObjB = 0;
        $sntcObjBDesc = 0;
    }

    $selVerb = %vrb;
    setCurrentVerb($SntcLine);
    redrawVerb();
}

// defaultAction uses waitForMessage -> script
function ResRoom::defaultAction(%vrb, %objA, %objB)
{
    switch$ (%vrb)
    {
        case "WalkTo":
            return;

        case "PickUp":
            if (%objA)// TOFIX is Pickable && getObjectVerbEntrypoint(%objA, InventoryObject))
            {
                // TOFIX startObject2(%objA, InventoryObject, [ InventoryObject, %objA ]);
                if ($VAR_RETURN)
                {
                    pickupObject($VAR_RETURN, InventoryItems);
                    // TOFIX setObjectClass(%objA, [ 0x80 + ClassUntouchable ]);
                    setObjectState(%objA, 0);
                    return;
                }
            }
            if (%objA)// TOFIX is Person) 
                egoSay("I don't need them.");
            else                  
                egoSay("I don't need that.");
            break;

        case "Use":
        case "UsedWith":
            if (%objA)// TOFIX is Person)
            {
                if (%objA == commanderZif || %objA == ensignZob)
                {
                    $VAR_EGO = %objA;
                    cameraFollowActor($VAR_EGO);
                }
                else egoSay("I can't just *use* someone.");
                break;
            }

            if (%objB)
            {
                //  TOFIX if (%vrb == Use && getObjectVerbEntrypoint(%objB, UsedWith))
                //  TOFIX { startObject2(%objB, UsedWith, [ UsedWith, %objB, %objA ]); break; }

                //  TOFIX if (%objB is Person) egoSay("I can't use that on someone!");
                //  TOFIX else                  egoSay("That doesn't work.");
            }
            else 
            {
                egoSay("I don't know how to operate that.");
            }
            break;

        case "Give":
            //  TOFIX if (%objB && %objB is Person) 
            //  TOFIX     egoSay("I don't think i should be giving this away.");
            //  TOFIX else                           
            //  TOFIX     egoSay("I can't do that !");
            break;

        case "LookAt":
            //  TOFIX if (%objA is Person) 
            //  TOFIX     egoSay("Some kind of lifeform.");
            //  TOFIX else                  
            //  TOFIX     egoSay("Looks pretty ordinary.");
            break;

        case "Move":
            //  TOFIX if (%objA is Person) 
            //  TOFIX     egoSay("Moving them would accomplish nothing.");
            //  TOFIX else                  
            //  TOFIX     egoSay("Moving that would accomplish nothing.");
            break;

        case "Open":
            // TOFIX if (%objA is Person) { egoSay("They don't seem to open."); break; }
            // TOFIX if (%objA is !Openable) { egoSay("That doesn't seem to open."); break; }

            setObjectState(%objA, !getObjectState(%objA));
            if (getObjectVerbEntrypoint(%objA, SetBoxes))
            {
                // TOFIX startObject2(%objA, SetBoxes, [ %vrb, %objA ]);
            }
            break;

        case "Smell":
            // TOFIX if (%objA is Person) egoSay("They have no odour.");
            // TOFIX else                  egoSay("That has no odour.");
            break;

        case "TalkTo":
            //  TOFIX if (%objA is Person) egoSay("I don't know how to communicate with them.");
            //  TOFIX else                  egoSay("I don't know how to communicate with that.");
            break;

        default:
            egoSay("Hmm. No.");
            break;
    }
    waitForMessage();
}

// sentenceHandler uses waits -> script
function ResRoom::sentenceHandler(%vrb, %objA, %objB)
{
    %owner = 0; %act = 0;

    if (%vrb == $SntcLine)
    {
        %vrb  = sntcVerb;
        %objA = sntcObjA;
        %objB = sntcObjB;
    }

    %owner = getObjectOwner(%objA);

    // Use/Give must own first
    while (false) // TOFIX isAnyOf(%vrb, [ Use, Give ]))
    {
        if (!isObject(%objB))
        {
            if (getObjectVerbEntrypoint(%objA, Preposition))
            {
                // TOFIX startObject2(%objA, Preposition, [ %vrb, %objA ]);
                if ($sntcPrepo) { setCurrentVerb($SntcLine); redrawVerb(); return; }
            }
            break;
        }

        if (%owner != $VAR_EGO)
        {
            if ($tryPick == %objA) { $tryPick = 0; return; }

            if (getObjectVerbEntrypoint(%objA, InventoryObject))
            {
                // TOFIX startObject2(%objA, InventoryObject, [ InventoryObject, %objA ]);
                $tryPick = $VAR_RETURN;
            }
            else $tryPick = %objA;

            doSentence(%vrb, $tryPick, 0, %objB);
            doSentence(PickUp, %objA, 0, 0);
            return;
        }
        else $tryPick = 0;
        break;
    }

    // Walk near room object/actor if needed
    if (%objA <= 0xF || %owner == 0xF)
    {
        walkActorToObj($VAR_EGO, %objA, 0);
        waitForActor($VAR_EGO);
        if (%objA <= 0xF) actorFace($VAR_EGO, %objA);
    }
    else if (%objB) if (%objB <= 0xF || getObjectOwner(%objB) == 0xF)
    {
        walkActorToObj($VAR_EGO, %objB, 0);
        waitForActor($VAR_EGO);
        if (%objB <= 0xF) actorFace($VAR_EGO, %objB);
    }

    // Resolve actor proxy object if needed
    if (%objA <= 0xF) if ($actorObject[%objA]) { %act = %objA; %objA = $actorObject[%objA]; }

    // Dispatch to object verb or fallback
    if (getObjectVerbEntrypoint(%objA, %vrb))
    {
        // TOFIX startObject(2, %objA, %vrb, [ %vrb, %objA, %objB ]);
        do { breakScript();  } while (!$cursorOn);
    }
    else
    {
        %this.defaultAction(%vrb, (%act ? %act : %objA), %objB);
    }

    // If verb needs objB, stop now
    if ($sntcPrepo && !%objB) return;

    %this.resetSntc(0);
}

// keyboardHandler uses waitForMessage/restart -> script
function ResRoom::keyboardHandler(%key)
{
    switch$ (%key)
    {
        case "o":
            egoSay("Hooo");
            break;

        case "r":
            egoSay("Let's restart."); waitForMessage();
            restartGame();
            break;

        case "q":
            shutdown();
            break;
    }
}

// =========================
// Input (mouse/keyboard dispatcher) — no waits -> function
// =========================
function ResRoom::inputHandler(%area, %cmd, %btn)
{
    %invCount = 0; $invOffsetMax = 0;

    echoBegin();
    echo("Area=%" @ %area @ " cmd=%" @ %cmd @ " button=%" @ %btn);
    echoEnd();

    egoPrintBegin();
    egoPrintOverhead();
    actorPrintEnd();

    if (%area == 4) { %this.keyboardHandler(%cmd); return; }

    if (false)// TOFIXisAnyOf(%cmd, [ Give, PickUp, Use, Open, LookAt, Smell, TalkTo, Move ]))
    {
        %this.resetSntc(%cmd);
        return;
    }

    if (%cmd == invUp || %cmd == invDown)
    {
        %invCount = getInventoryCount($VAR_EGO);
        $invOffset += ((%cmd == invUp) ? -1 : 1) * INVENTORY_COL;
        $invOffsetMax = ((%invCount + INVENTORY_COL - 1) / INVENTORY_COL - INVENTORY_LINE) * INVENTORY_COL;

        if ($invOffset > $invOffsetMax) $invOffset = $invOffsetMax;
        if ($invOffset < 0)             $invOffset = 0;

        echo("Inventory offset: %i{" @ $invOffset @ "}");
        Verbs::showVerbs(1);
        %this.inventoryHandler(0);
        return;
    }

    // Stop current sentence and (re)start mouse watch to refresh sentence line
    stopSentence();
    %this.mouseWatch();

    // RMB with no selection cancels walk
    if (%btn == 2 && !($sntcPrepo ? %sntcObjB : %sntcObjA))
    {
        setCurrentActor($VAR_EGO);
        setActorStanding();
        %this.resetSntc(0);
        return;
    }

    // Clicked an object (room or inventory)
    if ($sntcPrepo ? sntcObjB : sntcObjA)
    {
        if (%cmd) $selVerb = sntcVerb;               // keep displayed verb
        if (%btn == 2 && $altVerb) { $selVerb = $altVerb; %this.mouseWatch(); }
        doSentence(sntcVerb, sntcObjA, 0, sntcObjB); // queue sentence
        return;
    }

    // Non-room area click: ignore
    if (%area != 2) return;

    // Clicked on empty room space: walk there
    if ($selVerb) %this.resetSntc(0);
    walkActorTo($VAR_EGO, $VAR_VIRT_MOUSE_X, $VAR_VIRT_MOUSE_Y);
}

// =========================
// Inventory helpers (no waits) -> function
// =========================
function ResRoom::setInventoryIcon(%icon, %slot)
{
    setCurrentVerb(%slot);
    setVerbObject(%icon, InventoryItems);
    redrawVerb();
}

function ResRoom::inventoryHandler(%obj)
{
    %i = 0; %count = getInventoryCount($VAR_EGO);
    echo(%count @ " obj in inv");

    for (%i = 0; %i < INVENTORY_SLOTS; %i++)
    {
        if (%i + $invOffset < %count)
        {
            %obj = findInventory($VAR_EGO, %i + 1 + $invOffset);
            %this.setInventoryIcon(%obj, $invSlot0 + %i);
        }
        else
        {
            setCurrentVerb($invSlot0 + %i);
            setVerbNameString(0);
            redrawVerb();
        }
    }
}

// =========================
// Hooks: pre-entry & quit (no waits)
// =========================
function ResRoom::preEntry(%this) { Actors::loadObjects(); }
function ResRoom::quit(%this)     { shutdown(); }

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
    $VAR_GUI_COLORS[0] = 0x00,0x00,0x43,0x00,0xD7,0x34,0x52,0x90,0x00,0x6A,
                          0x06,0x1A,0xD5,0xE5,0xE3,0xE5,0xE3,0xE5,0xE3,0xE5,
                          0xE3,0x00,0x00,0x00,0x00,0x14,0xD7,0xE5,0xE3,0xE5,
                          0xE3,0x37,0x1C,0xE5,0xE3,0xE5,0xE3,0x14,0xD7,0xE5,
                          0xE3,0xE5,0xE3,0x00,0x00,0x00,0x00,0x00,0x00,0x00;

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
    $VAR_VERB_SCRIPT          = ResRoom, inputHandler;
    $VAR_SENTENCE_SCRIPT      = ResRoom, sentenceHandler;
    $VAR_INVENTORY_SCRIPT     = ResRoom, inventoryHandler;
    $VAR_CUTSCENE_START_SCRIPT= ResRoom, cutsceneStart;
    $VAR_CUTSCENE_END_SCRIPT  = ResRoom, cutsceneEnd;
    $VAR_PRE_ENTRY_SCRIPT     = ResRoom, preEntry;

    // Preload rooms kept resident
    loadRoom(ResRoom);   lockRoom(ResRoom);
    loadRoom(OfficeRoom);lockRoom(OfficeRoom);

    // Screen height must match room
    setScreen(0,144);

    // Actor<->object mapping array
    dimInt($actorObject, 0x10);

    // Init default charset
    initCharset(chtest);

    // Cursor & mouse tracking
    %this.showCursor();
    %this.mouseWatch();

    // Talking print slots
    printBegin(); printCenter(); printOverhead(); printEnd();

    // Boot path
    switch (%bootParam)
    {
        case 2:
            setObjectState(OfficeRoom->exitToSecretRoom, 7);
            $OfficeRoom::didOfficeIntro = 1;
            startRoom(OfficeRoom);

        case 1:
            $OfficeRoom::didOfficeIntro = 1;
            startRoom(OfficeRoom);

        default:
            screenEffect(0x0005);
            startRoom(TitleScreen);
    }
}
