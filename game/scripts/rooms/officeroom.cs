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

// =========================
// OfficeRoom globals / flags
// =========================
$OfficeRoom::hasSeenBullets        = 0;
$OfficeRoom::hasTriedToMovePlant   = 0;
$OfficeRoom::hasTalkedAboutPlant   = 0;
$OfficeRoom::knowsCarol            = 0;
$OfficeRoom::askedPassageway       = 0;
$OfficeRoom::initedGame            = 0;

$OfficeRoom::hasSmelledPlate       = 0;
$OfficeRoom::hasPressedPlate       = 0;
$OfficeRoom::hasTalkedAboutPlate   = 0;

$OfficeRoom::didOfficeIntro        = 0;

// =========================
// Room resources
// =========================
new Sound(Office_beamedSnd)       { path = "sounds/beamed.wav"; };
new Sound(Office_phoneSnd)        { path = "sounds/phone.wav"; };
new Sound(Office_openCabinetSnd)  { path = "sounds/open_cabinet.wav"; };
new Sound(Office_closeCabinetSnd) { path = "sounds/close_cabinet.wav"; };
new Sound(Office_switchSnd)       { path = "sounds/switch.wav"; };
new Sound(Office_movePlantSnd)    { path = "sounds/move_plant.wav"; };
new Sound(Office_openDoorSnd)     { path = "sounds/open_door.wav"; };
new Sound(Office_openedDoorSnd)   { path = "sounds/opened_door.wav"; };

// =========================
// Room: OfficeRoom
// (Actors are shown by the room; objects are also displayables.)
// =========================
new Room(OfficeRoom)
{
    class = BaseRoom;
    image = "graphics/rooms/back01_merged.bmp";
    boxFile = "graphics/rooms/back01.box";
    zPlane[0] = "graphics/rooms/back01_mask1.bmp";
    zPlane[1] = "graphics/rooms/back01_mask2.bmp";
    zPlane[2] = "graphics/rooms/back01_mask3.bmp";

    // ---------- Objects ----------
    // Order matters: bullets before plant to draw above it.
    new RoomObject(ObjBullets)
    {
        anchorPoint = 112, 80;dir = EAST;
        displayText = "ammunition";
        className = Pickable;  // mirrors { Pickable }
        stateOffset[0] = -8, 16;
        stateBitmap[0] = "graphics/background_items/bullets.bmp";
        state = 1;
        trans = 0;

        // Parent gating (exists only when plant is state 2)
        parent       = plant;
        parent_state = 2;

        new RoomObjectState()
        {
           hotSpot = -8, 16;
           image = "graphics/background_items/bullets.bmp";
        };
    };

    new RoomObject(ObjPlant)
    {
        anchorPoint = 104, 48;dir = EAST;
        displayText = "plant";
        state = 1;
        
        new RoomObjectState()
        {
           hotSpot = 0, 48;
           image = "graphics/background_items/plant_unmoved.bmp";

           zPlane[0] = "";
           zPlane[1] = "graphics/background_items/plant_mask2.bmp";
           zPlane[2] = "graphics/background_items/plant_unmoved_mask3.bmp";
         };


        new RoomObjectState()
        {
           hotSpot = 0, 48;
           image = "graphics/background_items/plant_moved.bmp";

           zPlane[0] = "";
           zPlane[1] = "graphics/background_items/plant_mask2.bmp";
           zPlane[2] = "graphics/background_items/plant_moved_mask3.bmp";
        };
    };

    new RoomObject(ObjCabinetDrawer)
    {
        anchorPoint = 128, 72;dir = NORTH;
        displayText = "cabinet";
        className = Openable;
        state = 1;

        new RoomObjectState()
        {
           hotSpot = 30, 16;
           image = "graphics/background_items/drawer_closed.bmp";
         };

        new RoomObjectState()
        {
           hotSpot = 30, 16;
           image = "graphics/background_items/drawer_open.bmp";
         };
    };

    new RoomObject(ObjPlate)
    {
        anchorPoint = 248, 64;dir = EAST;
        contentSize = 8, 16; hotSpot = -14, 30;
        displayText = "plate";
    };

    new RoomObject(exitToSecretRoom)
    {
        anchorPoint = 248, 32;dir = EAST;
        displayText = "secret room";
        state = 1;

        new RoomObjectState()
        {
           hotSpot = 8, 78;
           image = "graphics/door/door_closed.bmp";
         };

        new RoomObjectState()
        {
           hotSpot = 8, 78;
           image = "graphics/door/door_opening_01.bmp";
         };

        new RoomObjectState()
        {
           hotSpot = 8, 78;
           image = "graphics/door/door_opening_02.bmp";
         };


        new RoomObjectState()
        {
           hotSpot = 8, 78;
           image = "graphics/door/door_opening_03.bmp";
         };


        new RoomObjectState()
        {
           hotSpot = 8, 78;
           image = "graphics/door/door_opening_04.bmp";
         };


        new RoomObjectState()
        {
           hotSpot = 8, 78;
           image = "graphics/door/door_opening_05.bmp";
         };


        new RoomObjectState()
        {
           hotSpot = 8, 78;
           image = "graphics/door/door_open.bmp";
         };
    };

    new RoomObject(lightSwitch)
    {
        anchorPoint = 34, 71;contentSize = 7, 9;
        displayText = "light switch"; dir = WEST;
        hotspot = 20, 40;
    };

    new RoomObject(powerSocket)
    {
        anchorPoint = 220, 84;contentSize = 11, 6;
        displayText = "power socket"; dir = NORTH;
        hotspot = 5, 20;
    };

    new RoomObject(bulletinBoard)
    {
        anchorPoint = 65, 39;contentSize = 14, 24;
        displayText = "bulletin board"; dir = WEST;
        hotspot = 20, 46;
    };
};


// =========================
// Entry / cutscene & helpers
// =========================

// Entry uses waits/cutscene -> script
function OfficeRoom::onEntry(%this)
{
    %firstInit = !$OfficeRoom::initedGame;
    %quickInit = (%firstInit && $OfficeRoom::didOfficeIntro);

    if (!$OfficeRoom::initedGame)
    {
        echo("Initing game...");
        $OfficeRoom::initedGame = 1;
        Verbs.setupVerbs();
        Actors.setupActors();
    }
    else
    {
        $VAR_EGO.setPosition(296,110, OfficeRoom);
        $VAR_EGO.walkTo(250,110);
        if (lightSwitch.state == 1)
        {
            setRoomRGBIntensity(143,123,119,0,255);
        }
        return;
    }

    if (!$OfficeRoom::didOfficeIntro)
    {
        beginCutscene(2);
        //{
            $OfficeRoom::didOfficeIntro = 1;

            // try { ... }
            delayFiber(150);
            ensignZob.putAt(   170,110, OfficeRoom);
            commanderZif.putAt(200,120, OfficeRoom);
            Office_beamedSnd.play();
            commanderZif.animate(beam);
            ensignZob.animate(  beam);
            delayFiber(150);

            setCurrentActor(ensignZob);
            ensignZob.setStanding();

            commanderZif.say("Hmmm...");
            waitForMessage();
            Actors.setZifOnThePhone();
            Office_phoneSnd.play();
            commanderZif.animate(lookAround);
            delayFiber(30);
            commanderZif.say("Commander's log, star date 432.1");
            waitForMessage();
            commanderZif.say("Arrival complete on the planet of mostly water.");
            waitForMessage();

            // NOTE: "init" contains the walk-in anim here.
            setCurrentActor(carol);
            carol.putAt( 76,98, OfficeRoom);
            delayFiber(120);
            setActorStanding();
            carol.say("Are you done in here?");
            waitForMessage();

            actorFace(commanderZif, carol);
            actorFace(ensignZob,   carol);
            delayFiber(50);
            commanderZif.say("An indigenous lifeform...");
            waitForMessage();

            ensignZob.walkTo( 105,105); waitForActor(ensignZob);
            commanderZif.say("Ensign Zob and I shall proceed to locate the artifact.");
            waitForMessage();
            commanderZif.say("We shall eliminate any resistance we encounter.");
            waitForMessage();

            commanderZif.walkTo( 160,120); waitForActor(commanderZif);
            commanderZif.say("This appears to be a crude society.");
            waitForMessage();
            commanderZif.say("The defenses of the compound were negligible.");
            waitForMessage();
            commanderZif.say("I expect the relic to be in our possession shortly.");
            waitForMessage();

            Actors.setZifOffThePhone();
            ensignZob.say("What of this lifeform, Sir?");
            waitForMessage();
            commanderZif.say("Proceed with caution.");
            waitForMessage();
            commanderZif.say("That may be some kind of weapon.");
            waitForMessage();

            ensignZob.walkTo( 115,120); waitForActor(ensignZob);
            ensignZob.say("I'll begin my search."); waitForMessage();
            commanderZif.say("Keep me updated, ensign."); waitForMessage();

            // override { ... }
            if ($VAR_OVERRIDE)
            {
                stopTalking();
                carol.putAt(        76, 98, OfficeRoom);
                commanderZif.putAt(160,120, OfficeRoom);
                ensignZob.putAt(   115,120, OfficeRoom);
                setActorStanding();
                carol.animate(stand);
                Actors.setZifOffThePhone();
            }
        //}
        endCutscene();
    }

    if (%quickInit)
    {
        carol.putAt(        76, 98, OfficeRoom);
        commanderZif.putAt(160,120, OfficeRoom);
        ensignZob.putAt(   115,120, OfficeRoom);
        carol.animate(stand);
    }

    if (%firstInit)
    {
        Verbs.showVerbs(1);
        delayFiber(getRandomNumberRange(20,60));
        Actors.startRoaming(commanderZif, 20,260, 105,130);
    }
}

// Local setter helpers (no waits)
function OfficeRoom::setTalkedAboutPlant(%this) { $OfficeRoom::hasTalkedAboutPlant = 1; }
function OfficeRoom::setTalkedAboutPlate(%this) { $OfficeRoom::hasTalkedAboutPlate = 1; }

// Carol dialog (uses waits/cutscenes) -> script
function OfficeRoom::zobTalkToCarol(%this)
{
    %sentence = "";

    beginCutscene(1);
    //{
        if (!$OfficeRoom::knowsCarol)
        {
            egoSay("Speak, lifeform!"); waitForMessage();
            egoSay("Is this translation matrix working?"); waitForMessage();
            carol.say("You're that guy from accounts, right?"); waitForMessage();
        }
        else
        {
            egoSay("Greetings, Carol."); waitForMessage();
            carol.say("You again?"); waitForMessage();
        }
    //}
    endCutscene();

    while (true)
    {
        
        if (!$OfficeRoom::knowsCarol)
        {
            Dialog.dialogAdd("What is your designation?");
        }
        else
        {
            Dialog.dialogAdd("");
        }
        
        if (!$OfficeRoom::askedPassageway) 
        {
            Dialog.dialogAdd("What is located through that passageway?");
        }
        else
        {
            Dialog.dialogAdd("");
        }

        Dialog.dialogAdd("Stand aside, I need to pass!");
        Dialog.dialogAdd("Look out behind you, a three-headed monkey!");

        
        if ($OfficeRoom::hasTriedToMovePlant && !$OfficeRoom::hasTalkedAboutPlant)
        {
            Dialog.dialogAdd("May I move this plant?");
        }
        else
        {
            Dialog.dialogAdd("");
        }

        Dialog.dialogAdd("As you were.");
        

        Dialog.dialogStart($ZOB_DIM_COLOR, $ZOB_COLOR);
        do { breakFiber(); } while ($selectedSentence < 0);
        Dialog.dialogHide();

        beginCutscene();
        //{
            %chosen = Dialog.dialogList[$selectedSentence];
            egoSay(%chosen); waitForMessage();

            switch ($selectedSentence)
            {
                case 0:
                    carol.say("What you on about?"); waitForMessage();
                    egoSay("How are you addressed by a superior officer?"); waitForMessage();
                    carol.say("Carol."); waitForMessage();
                    egoSay("Greetings, Carol."); waitForMessage();
                    carol.displayText = "Carol";
                    $OfficeRoom::knowsCarol = 1;
                    setCurrentActor($VAR_EGO);

                case 1:
                    carol.say("Err, reception, canteen..."); waitForMessage();
                    egoSay("Any stolen relics within those realms?"); waitForMessage();
                    carol.say("Do you mean that new tea maker they installed?"); waitForMessage();
                    egoSay("No."); waitForMessage();
                    carol.say("Dunno then."); waitForMessage();
                    $OfficeRoom::askedPassageway = 1;

                case 2:
                    carol.say("I just cleaned down there, you'll get the floor dirty."); waitForMessage();
                    egoSay("In which case I shall remain here."); waitForMessage();

                case 3:
                    carol.say("Yeah, that's Terry."); waitForMessage();
                    carol.say("He works in HR.");   waitForMessage();

                case 5:
                    // "As you were." -> exit loop after wait below

                case 4:
                    carol.say("I haven't cleaned down there yet."); waitForMessage();
                    egoSay("Then by moving the plant I am providing assistance."); waitForMessage();
                    carol.say("Oh yeah, go on then."); waitForMessage();
                    $OfficeRoom::hasTalkedAboutPlant = 1;
            }

            waitForMessage();
        //}
        endCutscene();

        Dialog.dialogClear(1);
        if ($selectedSentence == 5) 
        {   
            break;
        }
    }

    Dialog.dialogEnd();
}


// =========================
// Verb handlers
// =========================

// bullets: simple handlers (no waits) -> function
function ObjBullets::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("A small box of projectiles.");
}

function ObjBullets::getPreposition(%this, %verb)
{
    if (%verb.internalName $= "Give") return "to";
    else                 return "with";
}

function ObjBullets::onPickUp(%this, %verb, %objA, %objB)
{
    %this.doPickup(InvBullets);
}

// plant: uses cutscenes/waits -> script
function ObjPlant::onLookAt(%this, %verb, %objA, %objB)
{
    if (!$OfficeRoom::hasSeenBullets)
    {
        $OfficeRoom::hasSeenBullets = 1;
        egoSay("There is a small object behind this plant beyond reach.");
    }
    else
    {
        egoSay("This plant appears to be undergoing photosynthesis.");
    }
}

function ObjPlant::onMove(%this, %verb, %objA, %objB)
{
    if (!$OfficeRoom::hasSeenBullets)
    {
        egoSay("Moving this would accomplish nothing.");
        return;
    }

    if (!$OfficeRoom::hasTalkedAboutPlant)
    {
        beginCutscene(0);
        //{
            $VAR_EGO.animate(raiseArm);
            delayFiber(20);
            carol.say("Hey, I haven't cleaned there yet.");
            $OfficeRoom::hasTriedToMovePlant = 1;
            waitForMessage();
            $VAR_EGO.animate(lowerArm);
            delayFiber(30);
        //}
        endCutscene();
        return;
    }

    if (%this.state == 1)
    {
        beginCutscene(0);
        //{
            $VAR_EGO.animate(raiseArm);
            Office_movePlantSnd.play();
            delayFiber(20);
            %this.state = 2;
            delayFiber(30);
            $VAR_EGO.animate(lowerArm);
            delayFiber(30);
        //}
        endCutscene();

        egoSay("There is a small box down here.");
    }
    else
    {
        egoSay("It's fine over there.");
    }
    return;
}

function ObjCabinetDrawer::onLookAt(%this, %verb, %objA, %objB)
{
    if (%this.state < 2)
    {
        egoSay("Some kind of containing vessel for multiple instances of parchment.");
        return;
    }
    else
    {
        // Fallback to pickup
        %this.onPickUp(%this, %verb, %objA, %objB);
    }
}

function ObjCabinetDrawer::onPickUp(%this, %verb, %objA, %objB)
{
    if (%this.state < 2)
    {
        %this.onDefaultAction(%verb,%objA,%objB);
        return;
    }

    if (InventoryItems->gun.owner == 0)
    {
        beginCutscene(0);
        //{
            egoSay("I'll look in here I think."); waitForMessage();

            $VAR_EGO.animate(raiseArm); delayFiber(20);
            $VAR_EGO.animate(lowerArm); delayFiber(20);

            egoSay("There appears to be a small sidearm in this container.");
            $VAR_EGO.pickupObject(InvGun);
            waitForMessage();

            $VAR_EGO.animate(raiseArm); delayFiber(20);
            $VAR_EGO.animate(lowerArm); delayFiber(20);

            egoSay("And a plastic card containing some kind of circuitry.");
            $VAR_EGO.pickupObject(InvCard);
        //}
        endCutscene();
    }
    else
    {
        egoSay("Nothing else in here.");
    }
    return;
}

function ObjCabinetDrawer::onOpen(%this, %verb, %objA, %objB)
{
    if (!cabinetDrawer.state)
        Office_openCabinetSnd.play();
    else
        Office_closeCabinetSnd.play();

    Parent::onOpen(%this,%verb,%objA,%objB);
}

function ObjCabinetDrawer::isOpenable(%this)
{
    return true;
}

function CarolClass::onTalkTo(%this, %verb, %objA, %objB)
{
    OfficeRoom.zobTalkToCarol();
}

function CarolClass::onLookAt(%this, %verb, %objA, %objB)
{
    if (!$OfficeRoom::knowsCarol)
    {
        egoSay("A large carbon based mammal, it seems."); waitForMessage();
    }
    else
    {
        egoSay("It is Carol, one of the local lifeforms.");  waitForMessage();
    }
}

function CarolClass::onSmell(%this, %verb, %objA, %objB)
{
    egoSay("This creature has an overwhelming pungent smell.");
}

function CarolClass::onMove(%this, %verb, %objA, %objB)
{
    egoSay("The lifeform is surprisingly sturdy, I don't believe I can use force.");
}

function ObjPlate::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("It appears to be connected to the door mechanism.");
}

function ObjPlate::onSmell(%this)
{
    $OfficeRoom::hasSmelledPlate = 1;
    egoSay("I can smell the residue left by the indigenous lifeforms.");
    waitForMessage();
    egoSay("It must be some kind of pressure plate.");
}

function ObjPlate::onMove(%this)
{
    if (exitToSecretRoom.state == 7)
    {
        egoSay("The door is already open.");
        return;
    }

    if ($OfficeRoom::hasTalkedAboutPlate)
    {
        Actors::pauseRoaming(commanderZif);
        beginCutscene();
        //{
            // try { ... }
            commanderZif.walkTo( 267,116); waitForActor(commanderZif);
            commanderZif.say("I'll operate the one over here."); waitForMessage();
            delayFiber(20);
            $VAR_EGO.animate(    raiseArm);
            commanderZif.animate(raiseArm);
            delayFiber(30);
            Office_openDoorSnd.play();

            for (%i = 2; %i < 8; %i++)
            {
                delayFiber(10);
                exitToSecretRoom.state = %i;
            }

            Office_openedDoorSnd.play();
            $VAR_EGO.animate(    lowerArm);
            commanderZif.animate(lowerArm);
            delayFiber(30);

            commanderZif.say("Continue your investigation."); waitForMessage();
            commanderZif.walkTo( 200,120);

            // override { ... }
            if ($VAR_OVERRIDE)
            {
                stopTalking();
                commanderZif.putAt( 200,120, OfficeRoom);
                exitToSecretRoom.state = 7;
            }
        //}
        endCutscene();

        Actors::resumeRoaming(commanderZif);
        return;
    }

    if ($OfficeRoom::hasSmelledPlate)
    {
        beginCutscene(2);
        //{
            $OfficeRoom::hasPressedPlate = 1;
            $VAR_EGO.animate(raiseArm); delayFiber(30);
            $VAR_EGO.animate(lowerArm); delayFiber(30);
        //}
        endCutscene();
        egoSay("Nothing happened."); waitForMessage();
        egoSay("The two plates appear to be connected.");
        return;
    }

    egoSay("I don't know how to use that device.");
}

function ObjPlate::onUse(%this)
{
    %this.onMove(%verb, %objA, %objB);
}

// exitToSecretRoom: uses waitForActor -> script
function exitToSecretRoom::onWalkTo(%this, %verb, %objA, %objB)
{
    if (exitToSecretRoom.state == 1)
    {
        egoSay("It's closed.");
        return;
    }
    walkActorTo($VAR_EGO, 290,110); waitForActor($VAR_EGO);
    screenEffect(0x0083);
    startRoom(SecretRoom);
    return;
}

function lightSwitch::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("Some kind of power distribution grid.");
}

function lightSwitch::onUse(%this, %verb, %objA, %objB)
{
    if (lightSwitch.state == 1)
    {
        beginCutscene(0);
        //{
            egoSay("I'll turn this off."); waitForMessage();
            $VAR_EGO.animate(raiseArm); delayFiber(20);
            Office_switchSnd.play();
            setRoomRGBIntensity(143,123,119,0,255);
            $VAR_EGO.animate(lowerArm); delayFiber(20);
            lightSwitch.state = 2;
        //}
        endCutscene();
    }
    else
    {
        beginCutscene(0);
        //{
            egoSay("I'll turn this back on."); waitForMessage();
            $VAR_EGO.animate(raiseArm); delayFiber(20);
            Office_switchSnd.play();
            setRoomPalette(0);
            $VAR_EGO.animate(lowerArm); delayFiber(20);
            lightSwitch.state = 1;
        //}
        endCutscene();
    }
}

// powerSocket: simple -> function
function powerSocket::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("Some kind of power distribution grid.");
}

// bulletinBoard: simple -> function
function bulletinBoard::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("This parchment appears to contain some kind of glyphs.\w" @
           "Most are directed to an individual called Memorandum.");
}

function bulletinBoard::onSmell(%this, %verb, %objA, %objB)
{
    egoSay("The substance fused to this parchment possesses " @
           "an interesting aroma.");
}
