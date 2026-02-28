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
// SecretRoom globals / flags
// =========================
$SecretRoom::terminalActivated = 0;
$SecretRoom::cubeDisappeared   = 0;
$SecretRoom::hasShotAtNode     = 0;

// =========================
// Resources
// =========================
new Sound(Secret_shootCageSnd) { path = "sounds/shoot_cage.wav"; };
new Sound(Secret_shootDeadSnd) { path = "sounds/shoot_dead.wav"; };
new Sound(Secret_fallDeadSnd)  { path = "sounds/fall_dead.av"; };
new Sound(Secret_mwwwahSnd)    { path = "sounds/mwwwah_huh_huh.wav"; };

// =========================
// Room: SecretRoom
// =========================
new Room(SecretRoom)
{
    class = CoreRoom;
    
    image  = "graphics/rooms/back02_merged.bmp";
    boxd   = "graphics/rooms/back02.box";
    zplane =
        "graphics/rooms/back02_mask1.bmp",
        "graphics/rooms/back02_mask2.bmp",
        "graphics/rooms/back02_mask3.bmp"
    ;

    // Displayable objects
    new RoomObject(blueCup)
    {
        descName = "blue cup";
        dir  = SOUTH;
    };

    new RoomObject(computerTerminal)
    {
        basePosition = 176, 56;extent = 16, 24;
        hotspot = 30, 43;
        descName = "computer terminal";
        dir  = WEST;
    };

    new RoomObject(keySlot)
    {
        basePosition = 176, 80;extent = 16, 16;
        hotspot = 25, 30;
        descName = "key slot";
        dir  = WEST;
    };

    new RoomObject(node)
    {
        basePosition = 152, 16;extent = 16, 8;
        hs_x = -50; hs_y = 75;
        descName = "node";
        dir  = EAST;
    };

    new RoomObject(exitToOfficeRoom)
    {
        basePosition = 48, 32;extent = 32, 72;
        hotspot = 42, 68;
        descName = "room";
        dir  = WEST;
    };
};


// =========================
// Room entry / outro
// =========================

// No waits used -> function
function SecretRoom::onEntry()
{
    putActorAt($VAR_EGO, 40,100, SecretRoom);
    walkActorTo($VAR_EGO, 92,100);
}

// Uses waits/cutscene -> script
function SecretRoom::outro()
{
    %i = 0;

    Actors::stopRoaming(commanderZif);

    beginCutscene(1);
    //{
        // try { ... }
        egoSay("I'm having problems hitting the target, Sir."); waitForMessage();
        actorSay(commanderZif, "You're not a soldier. Give it here.");
        walkActorToObj(commanderZif, ensignZob, 0);
        waitForMessage();
        waitForActor(commanderZif);
        actorFace(commanderZif, ensignZob);
        actorFace(ensignZob, commanderZif);

        setObjectOwner(InventoryItems->gun, commanderZif);

        ensignZob.animate(  raiseArm);  delayFiber(15);
        commanderZif.animate(raiseArm); delayFiber(15);
        ensignZob.animate(  lowerArm);  delayFiber(15);
        commanderZif.animate(lowerArm); delayFiber(15);

        actorSay(commanderZif, "Right, stand back."); waitForMessage();
        ensignZob.walkTo( 210,120); waitForActor(ensignZob);

        walkActorToObj(commanderZif, node, 0); waitForActor(commanderZif);
        actorFace(ensignZob, commanderZif);
        actorSay(commanderZif, "I shall disable the suspension field thusly."); waitForMessage();

        delayFiber(100);
        startSound(Secret_shootCageSnd);
        commanderZif.animate(fireup);
        delayFiber(30);

        setCurrentActor(bluecupActor);
        for (%i = 11; %i > 0; %i--)
        {
            delayFiber(2);
            setActorElevation(%i * 5);
        }

        delayFiber(100);
        walkActorToObj(commanderZif, bluecupActor, 0); waitForActor(commanderZif);
        setCurrentActor(commanderZif);
        setActorDirection(180);
        delayFiber(100);

        // Move relic to inventory “room”
        bluecupActor.putAt( 0,0, InventoryItems);

        actorSay(commanderZif, "At last, the power of the relic is mine."); waitForMessage();
        egoSay("Expertly done, Sir."); waitForMessage();
        egoSay("Pass over the relic, and I shall place it in the containment vessel."); waitForMessage();

        commanderZif.walkTo( 100,115); waitForActor(commanderZif);
        actorFace(commanderZif, ensignZob);

        actorSay(commanderZif, "Never!"); waitForMessage();
        actorSay(commanderZif, "The relic shall never leave my side."); waitForMessage();
        egoSay("But commander, the power is too much for one man."); waitForMessage();
        egoSay("Yield it, before it destroys you!"); waitForMessage();

        actorSay(commanderZif, "Error, number one."); waitForMessage();
        actorSay(commanderZif, "Never give up the gun."); waitForMessage();

        startSound(Secret_shootDeadSnd);
        commanderZif.animate(firestraight);
        delayFiber(50);

        startSound(Secret_fallDeadSnd);
        ensignZob.animate(die);
        delayFiber(150);

        commanderZif.walkTo( 170,120); waitForActor(commanderZif);
        actorSay(commanderZif, "What a shame..."); waitForMessage();
        actorSay(commanderZif, "We could have ruled the cosmos together."); waitForMessage();

        setCurrentActor(commanderZif);
        setActorDirection(180);
        Actors::setZifOnThePhone();
        commanderZif.animate(lookAround); delayFiber(30);

        actorSay(commanderZif, "Commander's log, star date 432.2"); waitForMessage();
        actorSay(commanderZif, "I have obtained the stolen artifact."); waitForMessage();
        actorSay(commanderZif, "In a slight contradiction to my orders, I have decided to enslave this planet."); waitForMessage();
        actorSay(commanderZif, "%V{Secret_mwwwahSnd}Mwaaah huh huh huh huh huh huh..."); waitForMessage();
        Actors::setZifOffThePhone();

        // override { ... }
        if ($VAR_OVERRIDE)
        {
            actorSay(0xFF, ""); // stopTalking
            ensignZob.putAt(     276,110, SecretRoom);
            commanderZif.putAt(  170,120, SecretRoom);
            bluecupActor.putAt(     0,  0, InventoryItems);
            setCurrentActor(commanderZif);
            setActorDirection(180);
            Actors::setZifOffThePhone();
        }
    //}
    endCutscene();

    // Kill mouseWatch and queue quit
    ResRoom.stopMouseWatch();

    %this.gameOver = true;
}

function SecretRoom::inputHandler(%this, %area, %cmd, %btn)
{
    if (%this.gameOver)
    {
        // TODO: quit
    }
    else
    {
        return ResRoom.call(ResRoom.realInputHandler, %area, %cmd, %btn);
    }
}


// =========================
// Verb handlers
// =========================

// blueCup
function blueCup::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            egoSay("The holy artifact!");
            return;

        case "PickUp":
            if ($SecretRoom::cubeDisappeared)
                egoSay("The artifact is still being held in place.");
            else
                egoSay("The artifact is protected by some kind of forcefield container.");
            return;
    }
}

// computerTerminal (uses waits/cutscene)
function computerTerminal::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            egoSay("Looks pretty ordinary.");
            return;

        case "Use":
            if ($SecretRoom::terminalActivated && 
                !$SecretRoom::cubeDisappeared)
            {
                Actors::pauseRoaming(commanderZif);

                beginCutscene(2);
                //{
                    // try { ... }
                    $VAR_EGO.animate(raiseArm);  // (as in original)
                    delayFiber(25);
                    $VAR_EGO.animate(lowerArm);
                    delayFiber(25);

                    cubeActor.animate(dissolve);
                    delayFiber(50);

                    egoSay("Looks like I broke the encryption."); waitForMessage();

                    commanderZif.putAt( 40,100, SecretRoom);
                    commanderZif.walkTo( 95,100); waitForActor(commanderZif);
                    actorFace(commanderZif, $VAR_EGO);

                    actorSay(commanderZif, "Well done, ensign.");       waitForMessage();
                    actorSay(commanderZif, "Now retrieve the relic!");  waitForMessage();
                    egoSay("Small problem, sir.");                      waitForMessage();
                    actorSay(commanderZif, "What's that?");             waitForMessage();
                    egoSay("It is still held in place.");               waitForMessage();
                    actorSay(commanderZif, "Then find a way to bring it down."); waitForMessage();
                    egoSay("Aye, sir!");                                waitForMessage();

                    // override { ... }
                    if ($VAR_OVERRIDE)
                    {
                        actorSay(0xFF, ""); // stopTalking
                        commanderZif.putAt( 90,100, SecretRoom);
                        actorFace(commanderZif, $VAR_EGO);
                    }

                    // Post-actions (in original after block)
                    cubeActor.putAt( 0,0, InventoryItems);
                    $SecretRoom::cubeDisappeared = 1;
                    Actors::startRoaming(commanderZif, 80,220, 105,130);
                //}
                endCutscene();

                Actors::resumeRoaming(commanderZif);
            }
            else if ($SecretRoom::cubeDisappeared)
            {
                egoSay("The terminal has shut down.");
            }
            else
            {
                egoSay("The terminal is offline.");
            }
            return;
    }
}

// keySlot (uses cutscene/waits)
function keySlot::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            if ($SecretRoom::terminalActivated)
                egoSay("There is a keycard in this slot.");
            else
                egoSay("This looks like the recepticle for some kind of electronic key.");
            return;

        case "UsedWith":
            if (%objB == InventoryItems->card)
            {
                beginCutscene(0);
                //{
                    egoSay("I think this should activate the terminal."); waitForMessage();
                    $VAR_EGO.animate(raiseArm); delayFiber(25);
                    $VAR_EGO.animate(lowerArm);
                    $SecretRoom::terminalActivated = 1;
                    setObjectOwner(InventoryItems->card, 0);
                //}
                endCutscene();
            }
            return;
    }
}

// node
function node::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            egoSay("It appears to assist in suspending the artifact.");
            return;

        case "UsedWith":
            if (%objB == InventoryItems->gun)
            {
                if (getObjectState(InventoryItems->gun) != 2)
                {
                    egoSay("The weapon is not loaded.");
                    return;
                }
                if ($SecretRoom::cubeDisappeared != 1)
                {
                    egoSay("The suspension cage prevents a clear shot at the node.");
                    return;
                }

                beginCutscene(2);
                //{
                    $SecretRoom::hasShotAtNode = 1;
                    $VAR_EGO.animate(fireup);
                    startSound(Secret_shootCageSnd);
                    delayFiber(120);
                    egoSay("Missed!"); waitForMessage();
                //}
                endCutscene();

                egoSay("I am not familiar with the aiming reticule.");
                return;
            }
            return;
    }
}

// exitToOfficeRoom (uses waitForActor) -> script
function exitToOfficeRoom::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "WalkTo":
            walkActorTo($VAR_EGO, 50,100); waitForActor($VAR_EGO);
            screenEffect(0x0082);
            startRoom(OfficeRoom);
            return;
    }
}
