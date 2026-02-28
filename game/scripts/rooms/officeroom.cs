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
new Sound(Office_beamedSnd)       { path = "beamed.soun"; };
new Sound(Office_phoneSnd)        { path = "phone.soun"; };
new Sound(Office_openCabinetSnd)  { path = "open_cabinet.soun"; };
new Sound(Office_closeCabinetSnd) { path = "close_cabinet.soun"; };
new Sound(Office_switchSnd)       { path = "switch.soun"; };
new Sound(Office_movePlantSnd)    { path = "move_plant.soun"; };
new Sound(Office_openDoorSnd)     { path = "open_door.soun"; };
new Sound(Office_openedDoorSnd)   { path = "opened_door.soun"; };

// =========================
// Room: OfficeRoom
// (Actors are shown by the room; objects are also displayables.)
// =========================
new Room(OfficeRoom)
{
    class = CoreRoom;
    image   = "graphics/rooms/back01_merged.bmp";
    zplane  = "graphics/rooms/back01_mask1.bmp", "graphics/rooms/back01_mask2.bmp", "graphics/rooms/back01_mask2.bmp";
    boxd    = "graphics/rooms/back01.box";
    trans   = 0;

    // ---------- Objects ----------
    // Order matters: bullets before plant to draw above it.
    new RoomObject(bullets)
    {
        basePosition = 112, 80;dir = EAST;
        descName = "ammunition";
        className = Pickable;  // mirrors { Pickable }
        stateOffset[0] = -8, 16;
        stateBitmap[0] = "graphics/background_items/bullets.bmp";
        state = 1;
        trans = 0;

        // Parent gating (exists only when plant is state 2)
        parent       = plant;
        parent_state = 2;
    };

    new RoomObject(plant)
    {
        basePosition = 104, 48;dir = EAST;
        descName = "plant";
        stateOffset[0] = 0, 48;
        stateBitmap[0] = "graphics/background_items/plant_unmoved.bmp";
        stateZPlanes[0] = "", "graphics/background_items/plant_mask2.bmp", "graphics/background_items/plant_unmoved_mask3.bmp";

        stateOffset[1] = 0, 48;
        stateBitmap[1] = "graphics/background_items/plant_moved.bmp";
        stateZPlanes[1] = "", "graphics/background_items/plant_mask2.bmp", "graphics/background_items/plant_moved_mask3.bmp";
        
        state = 1;
    };

    new RoomObject(cabinetDrawer)
    {
        basePosition = 128, 72;dir = NORTH;
        descName = "cabinet";
        className = Openable;
        stateOffset[0] = 30, 16;
        stateBitmap[0] = "graphics/background_items/drawer_open.bmp";
        state = 0;
    };

    new RoomObject(carolObj)
    {
        descName = "Indigenous lifeform";
    };

    new RoomObject(plate)
    {
        basePosition = 248, 64;dir = EAST;
        extent = 8, 16; hs_x = -14; hs_y = 30;
        descName = "plate";
    };

    new RoomObject(exitToSecretRoom)
    {
        basePosition = 248, 32;dir = EAST;
        descName = "secret room";
        stateOffset[0] = 8, 78;
        stateBitmap[0] = "graphics/door/door_closed.bmp";
        stateOffset[1] = 8, 78;
        stateBitmap[1] = "graphics/door/door_opening_01.bmp";
        stateOffset[2] = 8, 78;
        stateBitmap[2] = "graphics/door/door_opening_02.bmp";
        stateOffset[3] = 8, 78;
        stateBitmap[3] = "graphics/door/door_opening_03.bmp";
        stateOffset[4] = 8, 78;
        stateBitmap[4] = "graphics/door/door_opening_04.bmp";
        stateOffset[5] = 8, 78;
        stateBitmap[5] = "graphics/door/door_opening_05.bmp";
        stateOffset[6] = 8, 78;
        stateBitmap[6] = "graphics/door/door_open.bmp";

        state = 1;
    };

    new RoomObject(lightSwitch)
    {
        basePosition = 34, 71;extent = 7, 9;
        descName = "light switch"; dir = WEST;
        hotspot = 20, 40;
    };

    new RoomObject(powerSocket)
    {
        basePosition = 220, 84;extent = 11, 6;
        descName = "power socket"; dir = NORTH;
        hotspot = 5, 20;
    };

    new RoomObject(bulletinBoard)
    {
        basePosition = 65, 39;extent = 14, 24;
        descName = "bulletin board"; dir = WEST;
        hotspot = 20, 46;
    };
};


// =========================
// Entry / cutscene & helpers
// =========================

// Entry uses waits/cutscene -> script
function OfficeRoom::entry(%this)
{
    %firstInit = !$OfficeRoom::initedGame;
    %quickInit = (%firstInit && $OfficeRoom::didOfficeIntro);

    if (!$OfficeRoom::initedGame)
    {
        $OfficeRoom::initedGame = 1;
        Verbs::setupVerbs();
        Actors::setupActors();
    }
    else
    {
        putActorAt($VAR_EGO, 296,110, OfficeRoom);
        walkActorTo($VAR_EGO, 250,110);
        if (getObjectState(lightSwitch) == 1)
            setRoomRGBIntensity(143,123,119,0,255);
        return;
    }

    if (!$OfficeRoom::didOfficeIntro)
    {
        beginCutscene(2);
        //{
            $OfficeRoom::didOfficeIntro = 1;

            // try { ... }
            delay(150);
            putActorAt(ensignZob,   170,110, OfficeRoom);
            putActorAt(commanderZif,200,120, OfficeRoom);
            startSound(Office_beamedSnd);
            animateActor(commanderZif, zif_anim_beam);
            animateActor(ensignZob,   zob_anim_beam);
            delay(150);

            setCurrentActor(ensignZob);
            setActorStanding();

            actorSay(commanderZif, "Hmmm...");
            waitForMessage();
            Actors::setZifOnThePhone();
            startSound(Office_phoneSnd);
            animateActor(commanderZif, zif_anim_lookAround);
            delay(30);
            actorSay(commanderZif, "Commander's log, star date 432.1");
            waitForMessage();
            actorSay(commanderZif, "Arrival complete on the planet of mostly water.");
            waitForMessage();

            setCurrentActor(carol);
            putActorAt(carol, 76,98, OfficeRoom);
            delay(120);
            setActorStanding();
            actorSay(carol, "Are you done in here?");
            waitForMessage();

            actorFace(commanderZif, carol);
            actorFace(ensignZob,   carol);
            delay(50);
            actorSay(commanderZif, "An indigenous lifeform...");
            waitForMessage();

            walkActorTo(ensignZob, 105,105); waitForActor(ensignZob);
            actorSay(commanderZif, "Ensign Zob and I shall proceed to locate the artifact.");
            waitForMessage();
            actorSay(commanderZif, "We shall eliminate any resistance we encounter.");
            waitForMessage();

            walkActorTo(commanderZif, 160,120); waitForActor(commanderZif);
            actorSay(commanderZif, "This appears to be a crude society.");
            waitForMessage();
            actorSay(commanderZif, "The defenses of the compound were negligible.");
            waitForMessage();
            actorSay(commanderZif, "I expect the relic to be in our possession shortly.");
            waitForMessage();

            Actors::setZifOffThePhone();
            actorSay(ensignZob, "What of this lifeform, Sir?");
            waitForMessage();
            actorSay(commanderZif, "Proceed with caution.");
            waitForMessage();
            actorSay(commanderZif, "That may be some kind of weapon.");
            waitForMessage();

            walkActorTo(ensignZob, 115,120); waitForActor(ensignZob);
            actorSay(ensignZob, "I'll begin my search."); waitForMessage();
            actorSay(commanderZif, "Keep me updated, ensign."); waitForMessage();

            // override { ... }
            if ($VAR_OVERRIDE)
            {
                stopTalking();
                putActorAt(carol,        76, 98, OfficeRoom);
                putActorAt(commanderZif,160,120, OfficeRoom);
                putActorAt(ensignZob,   115,120, OfficeRoom);
                setActorStanding();
                animateActor(carol, carol_anim_stand);
                Actors::setZifOffThePhone();
            }
        //}
        endCutscene();
    }

    if (%quickInit)
    {
        putActorAt(carol,        76, 98, OfficeRoom);
        putActorAt(commanderZif,160,120, OfficeRoom);
        putActorAt(ensignZob,   115,120, OfficeRoom);
        animateActor(carol, carol_anim_stand);
    }

    if (%firstInit)
    {
        Verbs::showVerbs(1);
        delay(getRandomNumberRange(20,60));
        Actors::startRoaming(commanderZif, 20,260, 105,130);
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
            actorSay(carol, "You're that guy from accounts, right?"); waitForMessage();
        }
        else
        {
            egoSay("Greetings, Carol."); waitForMessage();
            actorSay(carol, "You again?"); waitForMessage();
        }
    //}
    endCutscene();

    while (true)
    {
        
        if (!$OfficeRoom::knowsCarol) %sentence = "What is your designation?";
        Dialog::dialogAdd(%sentence);

        
        if (!$OfficeRoom::askedPassageway) %sentence = "What is located through that passageway?";
        Dialog::dialogAdd(%sentence);

         %sentence = "Stand aside, I need to pass!"; Dialog::dialogAdd(%sentence);
         %sentence = "Look out behind you, a three-headed monkey!"; Dialog::dialogAdd(%sentence);

        
        if ($OfficeRoom::hasTriedToMovePlant && !$OfficeRoom::hasTalkedAboutPlant)
            %sentence = "May I move this plant?";
        Dialog::dialogAdd(%sentence);

         %sentence = "As you were."; Dialog::dialogAdd(%sentence);
        

        Dialog::dialogStart(ZOB_DIM_COLOR, ZOB_COLOR);
        do { breakFiber(); } while ($selectedSentence < 0);
        Dialog::dialogHide();

        beginCutscene();
        //{
            %chosen = Dialog.dialogList[$selectedSentence];
            egoSay("%s{" @ %chosen @ "}"); waitForMessage();

            switch ($selectedSentence)
            {
                case 0:
                    actorSay(carol,"What you on about?"); waitForMessage();
                    egoSay("How are you addressed by a superior officer?"); waitForMessage();
                    actorSay(carol,"Carol."); waitForMessage();
                    egoSay("Greetings, Carol."); waitForMessage();
                    setObjectName($actorObject[carol], "Carol");
                    $OfficeRoom::knowsCarol = 1;
                    setCurrentActor($VAR_EGO);
                    break;

                case 1:
                    actorSay(carol,"Err, reception, canteen..."); waitForMessage();
                    egoSay("Any stolen relics within those realms?"); waitForMessage();
                    actorSay(carol,"Do you mean that new tea maker they installed?"); waitForMessage();
                    egoSay("No."); waitForMessage();
                    actorSay(carol,"Dunno then."); waitForMessage();
                    $OfficeRoom::askedPassageway = 1;
                    break;

                case 2:
                    actorSay(carol,"I just cleaned down there, you'll get the floor dirty."); waitForMessage();
                    egoSay("In which case I shall remain here."); waitForMessage();
                    break;

                case 3:
                    actorSay(carol,"Yeah, that's Terry."); waitForMessage();
                    actorSay(carol,"He works in HR.");   waitForMessage();
                    break;

                case 5:
                    // "As you were." -> exit loop after wait below
                    break;

                case 4:
                    actorSay(carol,"I haven't cleaned down there yet."); waitForMessage();
                    egoSay("Then by moving the plant I am providing assistance."); waitForMessage();
                    actorSay(carol,"Oh yeah, go on then."); waitForMessage();
                    $OfficeRoom::hasTalkedAboutPlant = 1;
                    break;
            }

            waitForMessage();
        //}
        endCutscene();

        Dialog::dialogClear(1);
        if (selectedSentence == 5) break;
    }

    Dialog::dialogEnd();
}


// =========================
// Verb handlers
// =========================

// bullets: simple handlers (no waits) -> function
function bullets::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            egoSay("A small box of projectiles.");
            return;

        case "Preposition":
            if (%verb $= "Give") $sntcPrepo[0] = "to";
            else                 $sntcPrepo[0] = "with";
            return;

        case "InventoryObject":
            return InventoryItems->bullets;
    }
}

// plant: uses cutscenes/waits -> script
function plant::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            if (!$OfficeRoom::hasSeenBullets)
            {
                $OfficeRoom::hasSeenBullets = 1;
                egoSay("There is a small object behind this plant beyond reach.");
            }
            else
                egoSay("This plant appears to be undergoing photosynthesis.");
            return;

        case "Move":
            if (!$OfficeRoom::hasSeenBullets)
            {
                egoSay("Moving this would accomplish nothing.");
                return;
            }

            if (!$OfficeRoom::hasTalkedAboutPlant)
            {
                beginCutscene(0);
                //{
                    animateActor($VAR_EGO, zob_anim_raiseArm);
                    delay(20);
                    actorSay(carol, "Hey, I haven't cleaned there yet.");
                    $OfficeRoom::hasTriedToMovePlant = 1;
                    waitForMessage();
                    animateActor($VAR_EGO, zob_anim_lowerArm);
                    delay(30);
                //}
                endCutscene();
                return;
            }

            if (getObjectState(plant) == 1)
            {
                beginCutscene(0);
                //{
                    animateActor($VAR_EGO, zob_anim_raiseArm);
                    startSound(Office_movePlantSnd);
                    delay(20);
                    setObjectState(plant, 2);
                    delay(30);
                    animateActor($VAR_EGO, zob_anim_lowerArm);
                    delay(30);
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
}

// cabinetDrawer: uses waits -> script
function cabinetDrawer::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            if (!getObjectState(cabinetDrawer))
            {
                egoSay("Some kind of containing vessel for multiple instances of parchment.");
                return;
            }
            // fallthrough to PickUp when open

        case "PickUp":
            if (!getObjectState(cabinetDrawer))
            {
                ResRoom::defaultAction(%verb,%objA,%objB);
                return;
            }

            if (getObjectOwner(InventoryItems->gun) == 0xF)
            {
                beginCutscene(0);
                //{
                    egoSay("I'll look in here I think."); waitForMessage();

                    animateActor($VAR_EGO, zob_anim_raiseArm); delay(20);
                    animateActor($VAR_EGO, zob_anim_lowerArm); delay(20);

                    egoSay("There appears to be a small sidearm in this container.");
                    pickupObject(InventoryItems->gun, InventoryItems);
                    waitForMessage();

                    animateActor($VAR_EGO, zob_anim_raiseArm); delay(20);
                    animateActor($VAR_EGO, zob_anim_lowerArm); delay(20);

                    egoSay("And a plastic card containing some kind of circuitry.");
                    pickupObject(InventoryItems->card, InventoryItems);
                //}
                endCutscene();
            }
            else
            {
                egoSay("Nothing else in here.");
            }
            return;

        case "Open":
            if (!getObjectState(cabinetDrawer))
                startSound(Office_openCabinetSnd);
            else
                startSound(Office_closeCabinetSnd);

            ResRoom::defaultAction(%verb,%objA,%objB);
            return;
    }
}

// carolObj: uses waits -> script
function carolObj::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "TalkTo":
            OfficeRoom::zobTalkToCarol();
            return;

        case "LookAt":
            if (!$OfficeRoom::knowsCarol)
            {
                egoSay("A large carbon based mammal, it seems."); waitForMessage();
            }
            else
            {
                egoSay("It is Carol, one of the local lifeforms.");  waitForMessage();
            }
            return;

        case "Smell":
            egoSay("This creature has an overwhelming pungent smell.");
            return;

        case "Move":
            egoSay("The lifeform is surprisingly sturdy, I don't believe I can use force.");
            return;
    }
}

// plate: uses waits/cutscenes -> script
function plate::onVerb(%this, %verb, %objA, %objB)
{
    %i = 0;

    switch$ (%verb)
    {
        case "LookAt":
            egoSay("It appears to be connected to the door mechanism.");
            return;

        case "Smell":
            $OfficeRoom::hasSmelledPlate = 1;
            egoSay("I can smell the residue left by the indigenous lifeforms.");
            waitForMessage();
            egoSay("It must be some kind of pressure plate.");
            return;

        case "Move" or "Use":
            if (getObjectState(exitToSecretRoom) == 7)
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
                    walkActorTo(commanderZif, 267,116); waitForActor(commanderZif);
                    actorSay(commanderZif, "I'll operate the one over here."); waitForMessage();
                    delay(20);
                    animateActor($VAR_EGO,     zob_anim_raiseArm);
                    animateActor(commanderZif, zif_anim_raiseArm);
                    delay(30);
                    startSound(Office_openDoorSnd);

                    for (%i = 2; %i < 8; %i++)
                    {
                        delay(10);
                        setObjectState(exitToSecretRoom, %i);
                    }

                    startSound(Office_openedDoorSnd);
                    animateActor($VAR_EGO,     zob_anim_lowerArm);
                    animateActor(commanderZif, zif_anim_lowerArm);
                    delay(30);

                    actorSay(commanderZif, "Continue your investigation."); waitForMessage();
                    walkActorTo(commanderZif, 200,120);

                    // override { ... }
                    if ($VAR_OVERRIDE)
                    {
                        stopTalking();
                        putActorAt(commanderZif, 200,120, OfficeRoom);
                        setObjectState(exitToSecretRoom, 7);
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
                    animateActor($VAR_EGO, zob_anim_raiseArm); delay(30);
                    animateActor($VAR_EGO, zob_anim_lowerArm); delay(30);
                //}
                endCutscene();
                egoSay("Nothing happened."); waitForMessage();
                egoSay("The two plates appear to be connected.");
                return;
            }

            egoSay("I don't know how to use that device.");
            return;
    }
}

// exitToSecretRoom: uses waitForActor -> script
function exitToSecretRoom::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "WalkTo":
            if (getObjectState(exitToSecretRoom) == 1)
            {
                egoSay("It's closed.");
                return;
            }
            walkActorTo($VAR_EGO, 290,110); waitForActor($VAR_EGO);
            screenEffect(0x0083);
            startRoom(SecretRoom);
            return;
    }
}

// lightSwitch: uses cutscene/waits -> script
function lightSwitch::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            egoSay("Some kind of power distribution grid.");

        case "Use":// TOFIX: though this worked?, "Move":
            if (!getObjectState(lightSwitch))
            {
                beginCutscene(0);
                //{
                    egoSay("I'll turn this off."); waitForMessage();
                    animateActor($VAR_EGO, zob_anim_raiseArm); delay(20);
                    startSound(Office_switchSnd);
                    setRoomRGBIntensity(143,123,119,0,255);
                    animateActor($VAR_EGO, zob_anim_lowerArm); delay(20);
                    setObjectState(lightSwitch, 1);
                //}
                endCutscene();
            }
            else
            {
                beginCutscene(0);
                //{
                    egoSay("I'll turn this back on."); waitForMessage();
                    animateActor($VAR_EGO, zob_anim_raiseArm); delay(20);
                    startSound(Office_switchSnd);
                    setRoomPalette(0);
                    animateActor($VAR_EGO, zob_anim_lowerArm); delay(20);
                    setObjectState(lightSwitch, 0);
                //}
                endCutscene();
            }
    }
}

// powerSocket: simple -> function
function powerSocket::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            egoSay("Some kind of power distribution grid.");
    }
}

// bulletinBoard: simple -> function
function bulletinBoard::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            egoSay("This parchment appears to contain some kind of glyphs.\w" @
                   "Most are directed to an individual called Memorandum.");

        case "Smell":
            egoSay("The substance fused to this parchment possesses " @
                   "an interesting aroma.");
    }
}
