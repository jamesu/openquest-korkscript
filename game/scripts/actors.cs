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
// Globals / constants
// =========================

// Roaming fields
$RM_STATE  = 0;
$RM_MIN_X  = 1;
$RM_MAX_X  = 2;
$RM_MIN_Y  = 3;
$RM_MAX_Y  = 4;
$RM_NUM    = 5;

// Roaming states
$RM_STOPPED = 0;
$RM_PAUSED  = 1;
$RM_RUNNING = 2;


// =========================
// Room: Actors
// =========================
new Room(Actors)
{
	roaming = "";

    // --- ACTORS (displayable) ---
    new Actor([ensignZob])      { className = EnsignZobClass; };
    new Actor([commanderZif])   { className = CommanderZifClass; };
    new Actor([carol])          { className = CarolClass; };
    new Actor([bluecupActor])   { className = BlueCupClass; };
    new Actor([cubeActor])      { className = CubeClass; };

};

// -------------------------
// Setup / loading
// -------------------------
function Actors::setup(%this)
{
    Actors.setupActors();
}

// This is called pre-entry to load the actor objects from this room
function Actors::loadObjects(%this)
{
    Actors.loadObjects();
}

// -------------------------
// Actor utility: Zif phone state
// -------------------------
function Actors::setZifOnThePhone(%this)
{
	%actor = %this->commanderZif;
    %actor.setStandFrame(zif_anim_standWithPhone);
    %actor.setInitFrame(zif_anim_standWithPhone);
    %actor.setTalkFrame(zif_anim_talkToPhoneStart, zif_anim_standWithPhone);
    %actor.setStanding();
}

function Actors::setZifOffThePhone(%this)
{
	%actor = %this->commanderZif;
    %actor.setStandFrame(zif_anim_stand);
    %actor.setInitFrame(zif_anim_init);
    %actor.setTalkFrame(zif_anim_talkStart, zif_anim_talkStop);
    %actor.setStanding();
}

// -------------------------
// Roaming scripts
// -------------------------

// Core roaming loop (uses waits) -> script
function Actors::roam(%this, %a)
{
    %event = 0; %dstX = 0; %dstY = 0; %paused = 0;
    echo("Roaming function for " @ %a @ " started");

    while( %this.roaming[%a, $RM_STATE] > $RM_STOPPED )
    {
        %paused = 0;

        // Pause if EGO == this actor or state <= PAUSED
        while( $VAR_EGO != %a && %this.roaming[%a, $RM_STATE] > $RM_PAUSED )
        {
            if(!%paused)
            {
                %paused = 1;
                echo("Roaming function for " @ %a @ " paused");
            }
            breakFiber();
        }
        if (%paused) echo("Roaming function for " @ %a @ " resumed");

        %event = getRandomNumber(1);

        switch$ (%event)
        {
            case 1:
                // pick a new reachable point not currently occupied
                do
                {
                    breakFiber();
                    %dstX = getRandom( %this.roaming[%a,$RM_MIN_X], %this.roaming[%a,$RM_MAX_X] );
                    %dstY = getRandom( %this.roaming[%a,$RM_MIN_Y], %this.roaming[%a,$RM_MAX_Y] );
                }
                while( %a.getRoom() == $VAR_ROOM && getActorAt(%dstX,%dstY) );

                echo("" @ %a @ " roam to " @ %dstX @ "x" @ %dstY @ "");
                %a.walkTo(%dstX, %dstY);
                break;
        }

        delay( getRandom(50,200) );
    }

    echo("Roaming function for " @ %a @ " finished");
}

// startRoaming uses delay/startScript -> script
function Actors::startRoaming(%this, %a, %x_min, %x_max, %y_min, %y_max)
{
    if(%a >= 0xF)
    {
        echo("Invalid actor id (" @ %a @ "), can't make it roam.");
        return;
    }

    echo("Starting roam function for " @ %a @ "");

    // lazy-dim nibble 2D
    /* TOFIX if(%this.roaming !$= "")
    {
        // Simulate dimNibble2(roaming,0xF,RM_NUM_FIELD)
        for (%i = 0; %i < 0xF; %i++)
            for (%j = 0; %j < $RM_NUM; %j++)
                %this.roaming[%i,%j] = 0;
    }*/

    %this.roaming[%a,$RM_MIN_X] = %x_min; %this.roaming[%a,$RM_MAX_X] = %x_max;
    %this.roaming[%a,$RM_MIN_Y] = %y_min; %this.roaming[%a,$RM_MAX_Y] = %y_max;

    if (%this.roaming[%a,$RM_STATE] > $RM_STOPPED)
    {
        %this.roaming[%a,$RM_STATE] = $RM_RUNNING;
    }
    else
    {
        %this.roaming[%a,$RM_STATE] = $RM_RUNNING;
        // TOFIX startScript(1, Actors.roam, [%a]);
    }
}

function Actors::pauseRoaming(%this, %a)
{
    if(!(%a < 0xF && %this.roaming !$= "")) return;
    if(!(%this.roaming[%a,$RM_STATE] > $RM_STOPPED)) return;

    echo("Pausing roam function for " @ %a @ "");
    %a.setStanding();
    %this.roaming[%a,$RM_STATE] = $RM_PAUSED;
}

function Actors::resumeRoaming(%this, %a)
{
    if(!(%a < 0xF && %this.roaming !$= "")) return;
    if(!(%this.roaming[%a,$RM_STATE] > $RM_STOPPED)) return;

    echo("Resuming roam function for " @ %a @ "");
    %this.roaming[%a,$RM_STATE] = $RM_RUNNING;
}

function Actors::stopRoaming(%this, %a)
{
    if(!(%a < 0xF && %this.roaming !$= "")) return;
    if(!(%this.roaming[%a,$RM_STATE] > $RM_STOPPED)) return;

    echo("Stoping roam function for " @ %a @ "");
    %a.setStanding();
    %this.roaming[%a,$RM_STATE] = $RM_STOPPED;
}

// -------------------------
// Actor setup
// -------------------------
function Actors::setupActors(%this)
{
    echo("setupActors()");

    // costumes
    %ensignZobCost    = "zob.cost";
    %commanderZifCost = "zif.cost";
    %carolCost        = "carol.cost";
    %bluecupCost      = "bluecup.cost";
    %cubeCost         = "cube.cost";

    // ---- ensign Zob ----
    %actor = %this->ensignZob;
    %actor.setCostume(%ensignZobCost);
    %actor.setName("Ensign Zob");
    %actor.setWalkSpeed(2,1);
    %actor.setTalkColor($ZOB_COLOR);
    %actor.setWidth(20);
    %actor.setAnimSpeed(4);
    // TOFIX %actor.setObjectClass([ 0x80 + Person ]);
    %actor.setPalette(29, 122); // scummvm fix

    // Ego
    $VAR_EGO = ensignZob;

    // ---- Commander Zif ----
    %actor = %this->commanderZif;
    %actor.setCostume(%commanderZifCost);
    %actor.setName("Commander Zif");
    %actor.setWalkSpeed(2,1);
    %actor.setTalkColor(ZIF_COLOR);
    %actor.setWidth(20);
    %actor.setAnimSpeed(4);
    // TOFIX %actor(commanderZif, [ 0x80 + Person ]);
    %actor.setPalette(29, 122); // scummvm fix

    // ---- Carol ----
    %actor = %this->carol;
    %actor.setCostume(%carolCost);
    %actor.setName("indigenous lifeform");
    %actor.setWidth(20);
    %actor.setIgnoreBoxes();
    %actor.setAnimSpeed(7);
    %actor.setTalkColor(CAROL_COLOR);
    // TOFIX %actor.setObjectClass([ 0x80 + Person ]);

    // ---- Blue cup (as actor) ----
    %actor = %this->bluecupActor;
    %actor.setCostume(%bluecupCost);
    %actor.setName("blue cup");
    %actor.setAnimSpeed(2);
    %actor.putActorAt(159,97,SecretRoom);
    %actor.setElevation(55);
    %actor.setWidth(0);

    // ---- Cube (as actor) ----
    %actor = %this->cubeActor;
    %actor.setCostume(%cubeCost);
    %actor.setName("cube");
    %actor.setAnimSpeed(2);
    // TOFIX %actor.setObjectClass([ 0x80 + ClassUntouchable ]);
    %actor.putActorAt(cubeActor,160,98,SecretRoom);
    %actor.setElevation(55);
    %actor.setWidth(0);

    // ---- Inventory ----
    pickupObject( InventoryItems->scanner );
    //pickupObject( InventoryItems->card );
    //pickupObject( InventoryItems->gun );
    //pickupObject( InventoryItems->bullets );
    $invOffset = 0;
}

function loadObjects(%this)
{
	// precache object images?
}

// -------------------------
// Dialog / interaction scripts (use waits)
// -------------------------

function Actors::zobTalkToZif(%this)
{
    %sentence = "";
    %commanderZif = %this->commanderZif;

    beginCutscene(1);
    //{
        commanderZif.face($VAR_EGO);
        commanderZif.say("Yes, ensign?");
        waitForMessage();
    //}
    endCutscene();

    while (true)
    {
    	%commanderZif = %this->commanderZif;
        %sentence = "What are your orders?";
        Dialog.dialogAdd("What are your orders?");

        if (OfficeRoom.hasPressedPlate && !OfficeRoom.hasTalkedAboutPlate)
            %sentence = "I require your assistance with this opening mechanism.";
        Dialog.dialogAdd(%sentence);

        %sentence = "I'll continue my search.";
        Dialog.dialogAdd(%sentence);

        Dialog.dialogStart($ZOB_DIM_COLOR, $ZOB_COLOR);
        do 
        {
        	breakFiber();
        }
        while ($selectedSentence < 0);

        Dialog.dialogHide();
        %forceStopTalking = false;

        try
        {
            beginCutscene();
        
            %chosen = Dialog.dialogList[$selectedSentence];
            egoSay("%s{" @ %chosen @ "");
            waitForMessage();

            switch ($selectedSentence)
            {
                case 0:
                    // try/override pattern preserved
                    if (true)
                    {
                        %commanderZif.say("Locate the stolen artifact.");
                        waitForMessage();
                        egoSay("Any ideas where?");
                        waitForMessage();
                        %commanderZif.say("Study your surroundings.");
                        waitForMessage();
                        %commanderZif.say("Use all the cunning and guile at your disposal.");
                        waitForMessage();
                        %commanderZif.say("Should you do this, you shall surely be successful.");
                        waitForMessage();
                        egoSay("Guile, right, aye sir.");
                        waitForMessage();
                        %commanderZif.say("Also get a gun with which to shoot things.");
                        waitForMessage();
                    }
                    %forceStopTalking = true;
                    break;

                case 2:
                    %commanderZif.say("Very good.");
                    break;

                case 1:
                    %commanderZif.say("Have you deciphered its secrets?");
                    waitForMessage();
                    egoSay("Yes, I believe two persons are required.");
                    waitForMessage();
                    %commanderZif.say("In which case I shall follow you, ensign.");
                    OfficeRoom.hasTalkedAboutPlate = 1;
                    break;
            }

            waitForMessage();

            endCutscene();
        }
        catch ($CUTSCENE_OVERRIDE)
        {
            if (%forceStopTalking)
            {
                stopTalking();
            }
        }

        Dialog.dialogClear(1);
        if ($selectedSentence == 2) break;
    }

    Dialog.dialogEnd();
}

function Actors::zobTalkToZifInSecretRoom(%this)
{
    %sentence = "";

    beginCutscene(1);
    //{
    	%actor = %this->commanderZif;
        %actor.face($VAR_EGO);
        %actor.say("Yes, ensign?");
        waitForMessage();
    //}
    endCutscene();

    while (true)
    {
        %sentence = "What now, Sir?";
        Dialog.dialogAdd(%sentence);

        %sentence = "I'll continue my search.";
        Dialog.dialogAdd(%sentence);

        Dialog.dialogStart($ZOB_DIM_COLOR, $ZOB_COLOR);
        do {
        	breakFiber(); 
        } while ($selectedSentence < 0);
        Dialog.dialogHide();

        %forceStopTalking = false;

        try
        {
            beginCutscene();

            %chosen = Dialog.dialogList[$selectedSentence];
            egoSay("%s{" @ %chosen @ "");
            waitForMessage();
        	%commanderZif = %this->commanderZif;

            switch ($selectedSentence)
            {
                case 0:
                    if (true)
                    {
                        %commanderZif.say("Get artifact.");
                        waitForMessage();
                        %commanderZif.say("We need to leave orbit quickly.");
                        waitForMessage();
                        egoSay("I'll see what I can do.");
                        waitForMessage();
                    }
                    
                    %forceStopTalking = true;
                    break;

                case 1:
                    %commanderZif.say("Very good.");
                    break;
            }

            waitForMessage();

            endCutscene();
        }
        catch ($CUTSCENE_OVERRIDE)
        {
            if (%forceStopTalking)
            {
                stopTalking();
            }
        }

        Dialog.dialogClear(1);
        if ($selectedSentence == 1) break;
    }

    Dialog.dialogEnd();
}

// =========================
// Verb handlers (RoomObjects)
// =========================

function CommanderZifClass::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "TalkTo":
            Actors.pauseRoaming(%this);
            if ($SecretRoom::cubeDisappeared)
            {
            	Actors.handle_zobTalkToZifInSecretRoom = runRoomScript(Actors, zobTalkToZifInSecretRoom);
                while (isFiberRunning(Actors.handle_zobTalkToZifInSecretRoom))
                {
                	breakFiber();
                }
                Actors.handle_zobTalkToZifInSecretRoom = 0;
            }
            else
            {
            	Actors.inst_zobTalkToZif = runRoomScript(Actors, zobTalkToZif);
                while (isFiberRunning(Actors.inst_zobTalkToZif.isRunning()))
                {
                	breakFiber();
                }
                Actors.inst_zobTalkToZif = 0;
            }
            Actors.resumeRoaming(%this);

        case "LookAt":
            egoSay("My commanding officer, Commander Zif.");

        case "Smell":
            Actors.pauseRoaming(%this);
            egoSay("Is that a new cologne, Sir?");
            waitForMessage();
            %this.say("I thought we got past these... feelings.");
            waitForMessage();
            egoSay("Sorry, Sir.");
            Actors.resumeRoaming(%this);
    }
}

function EnsignZobClass::onVerb(%this, %verb, %objA, %objB)
{
    // (empty in original)
}
