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
    class = BaseRoom;
    
    image  = "graphics/rooms/back02_merged.bmp";
    boxFile   = "graphics/rooms/back02.box";
    zPlane[0] = "graphics/rooms/back02_mask1.bmp";
    zPlane[1] = "graphics/rooms/back02_mask2.bmp";
    zPlane[2] = "graphics/rooms/back02_mask3.bmp";

    // Displayable objects
    new RoomObject(blueCup)
    {
        displayText = "blue cup";
        dir  = SOUTH;
    };

    new RoomObject(computerTerminal)
    {
        anchorPoint = 176, 56;contentSize = 16, 24;
        hotspot = 30, 43;
        displayText = "computer terminal";
        dir  = WEST;
    };

    new RoomObject(keySlot)
    {
        anchorPoint = 176, 80;contentSize = 16, 16;
        hotspot = 25, 30;
        displayText = "key slot";
        dir  = WEST;
    };

    new RoomObject(node)
    {
        anchorPoint = 152, 16;contentSize = 16, 8;
        hs_x = -50; hs_y = 75;
        displayText = "node";
        dir  = EAST;
    };

    new RoomObject(exitToOfficeRoom)
    {
        anchorPoint = 48, 32;contentSize = 32, 72;
        hotspot = 42, 68;
        displayText = "room";
        dir  = WEST;
    };
};


// =========================
// Room entry / outro
// =========================

// No waits used -> function
function SecretRoom::onEntry(%this)
{
    $VAR_EGO.putAt(40,100, SecretRoom);
    $VAR_EGO.walkTo(92,100);
}

// Uses waits/cutscene -> script
function SecretRoom::outro(%this)
{
    %i = 0;

    Actors.stopRoaming(commanderZif);

    beginCutscene(1);
    //{
        // try { ... }
        egoSay("I'm having problems hitting the target, Sir."); waitForMessage();
        commanderZif.say("You're not a soldier. Give it here.");
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

        commanderZif.say("Right, stand back."); waitForMessage();
        ensignZob.walkTo( 210,120); waitForActor(ensignZob);

        walkActorToObj(commanderZif, node, 0); waitForActor(commanderZif);
        actorFace(ensignZob, commanderZif);
        commanderZif.say("I shall disable the suspension field thusly."); waitForMessage();

        delayFiber(100);
        Secret_shootCageSnd.play();
        commanderZif.animate(fireup);
        delayFiber(30);

        for (%i = 11; %i > 0; %i--)
        {
            delayFiber(2);
            bluecupActor.setElevation(%i * 5);
        }

        delayFiber(100);
        walkActorToObj(commanderZif, bluecupActor, 0); waitForActor(commanderZif);
        setCurrentActor(commanderZif);
        commanderZif.setDirection($NORTH);
        delayFiber(100);

        // Move relic to inventory “room”
        bluecupActor.putAt( 0,0, InventoryItems);

        commanderZif.say("At last, the power of the relic is mine."); waitForMessage();
        egoSay("Expertly done, Sir."); waitForMessage();
        egoSay("Pass over the relic, and I shall place it in the containment vessel."); waitForMessage();

        commanderZif.walkTo( 100,115); waitForActor(commanderZif);
        actorFace(commanderZif, ensignZob);

        commanderZif.say("Never!"); waitForMessage();
        commanderZif.say("The relic shall never leave my side."); waitForMessage();
        egoSay("But commander, the power is too much for one man."); waitForMessage();
        egoSay("Yield it, before it destroys you!"); waitForMessage();

        commanderZif.say("Error, number one."); waitForMessage();
        commanderZif.say("Never give up the gun."); waitForMessage();

        Secret_shootDeadSnd.play();
        commanderZif.animate(firestraight);
        delayFiber(50);

        Secret_fallDeadSnd.play();
        ensignZob.animate(die);
        delayFiber(150);

        commanderZif.walkTo( 170,120); waitForActor(commanderZif);
        commanderZif.say("What a shame..."); waitForMessage();
        commanderZif.say("We could have ruled the cosmos together."); waitForMessage();

        setCurrentActor(commanderZif);
        setActorDirection(180);
        Actors.setZifOnThePhone();
        commanderZif.animate(lookAround); delayFiber(30);

        commanderZif.say("Commander's log, star date 432.2"); waitForMessage();
        commanderZif.say("I have obtained the stolen artifact."); waitForMessage();
        commanderZif.say("In a slight contradiction to my orders, I have decided to enslave this planet."); waitForMessage();
        commanderZif.say("%V{Secret_mwwwahSnd}Mwaaah huh huh huh huh huh huh..."); waitForMessage();
        Actors.setZifOffThePhone();

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
function blueCup::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("The holy artifact!");
}

function blueCup::onPickUp(%this, %verb, %objA, %objB)
{
    if ($SecretRoom::cubeDisappeared)
        egoSay("The artifact is still being held in place.");
    else
        egoSay("The artifact is protected by some kind of forcefield container.");
    return 0;
}

// computerTerminal (uses waits/cutscene)
function computerTerminal::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("Looks pretty ordinary.");
}

function computerTerminal::onUse(%this, %verb, %objA, %objB)
{
    if ($SecretRoom::terminalActivated && 
        !$SecretRoom::cubeDisappeared)
    {
        Actors.pauseRoaming(commanderZif);

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

            commanderZif.say("Well done, ensign.");       waitForMessage();
            commanderZif.say("Now retrieve the relic!");  waitForMessage();
            egoSay("Small problem, sir.");                      waitForMessage();
            commanderZif.say("What's that?");             waitForMessage();
            egoSay("It is still held in place.");               waitForMessage();
            commanderZif.say("Then find a way to bring it down."); waitForMessage();
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
            Actors.startRoaming(commanderZif, 80,220, 105,130);
        //}
        endCutscene();

        Actors.resumeRoaming(commanderZif);
    }
    else if ($SecretRoom::cubeDisappeared)
    {
        egoSay("The terminal has shut down.");
    }
    else
    {
        egoSay("The terminal is offline.");
    }
}

// keySlot (uses cutscene/waits)
function keySlot::onLookAt(%this, %verb, %objA, %objB)
{
    if ($SecretRoom::terminalActivated)
        egoSay("There is a keycard in this slot.");
    else
        egoSay("This looks like the recepticle for some kind of electronic key.");
}

function keySlot::onUsedWith(%this, %verb, %objA, %objB)
{
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
}

// node
function node::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("It appears to assist in suspending the artifact.");
}

function node::onUsedWith(%this, %verb, %objA, %objB)
{
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
            Secret_shootCageSnd.play();
            delayFiber(120);
            egoSay("Missed!"); waitForMessage();
        //}
        endCutscene();

        egoSay("I am not familiar with the aiming reticule.");
        return;
    }
}

function exitToOfficeRoom::onWalkTo(%this, %verb, %objA, %objB)
{
    $VAR_EGO.walkTo(50,100); 
    waitForActor($VAR_EGO);
    screenEffect(0x0082);
    startRoom(OfficeRoom);
}
