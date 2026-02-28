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
$MAX_DIALOG_LINES = 4;

// =========================
// Room: Dialog (no displayables needed)
// =========================
new Room(Dialog) {};

// =========================
// Dialog namespace functions
// =========================

// Reset the dialog list
function Dialog::dialogClear(%this, %kill)
{
    %i = 0; %j = "";

    if (%this.numDialog != 0)
    {
        if (%kill)
        {
            for (%i = 0; %i < %this.numDialog; %i++)
            {
                %this.dialogList[%i] = "";
            }
        }
    }
    %this.numDialog = 0;
    %this.numActiveDialog = 0;
}

// Add an entry to the dialog list
function Dialog::dialogAdd(%this, %str)
{
    if (%this.numDialog >= $MAX_DIALOG_SENTENCE)
    {
        echo("Too many sentences, can't add another one.");
        return;
    }

    %this.dialogList[%this.numDialog] = %str;
    %this.numDialog++;

    if (%str !$= "")
    {
        %this.numActiveDialog++;
    }
}

// Remove an entry; if kill != 0, undim the string too
function Dialog::dialogRemove(%this, %idx)
{
    if (%idx < 0 || %idx >= %this.numDialog)
    {
        echo("Dialog index out of range: " @ %idx @ ", can't remove.");
        return;
    }

    if (%this.dialogList[%idx] !$= "")
    {
        %this.numActiveDialog--;
    }

    // Shift the remainder down
    for (%i = %idx + 1; %i < %this.numDialog; %i++)
    {
        %this.dialogList[%i - 1] = %this.dialogList[%i];
    }

    %this.dialogList[%this.numDialog-1] = "";
    %this.numDialog--;
}

// Build/draw the visible dialog UI lines/arrows
function Dialog::showDialog(%this)
{
    %i = 0; %v = 0; %d = 0;
    %first = -1; %last = -1;
    %firstSentence = -1; %lastSentence = -1;

    // switch the charset
    initCharset(ResRoom.dialogCharset);

    // Create lines
    %v = 0; %d = 0;
    for (%i = 0; %i < %this.numDialog; %i++)
    {
        if (%this.dialogList[%i] $= "")
        {
            continue;
        }

        if (%firstSentence < 0) %firstSentence = %i;
        %lastSentence = %i;

        if (%v < %this.dialogOffset || %v >= %this.dialogOffset + $MAX_DIALOG_LINES)
        {
            %v++;
            continue;
        }

        if (%first < 0) %first = %i;
        %last = %i;


        setCurrentVerb($dialogVerb0 + %d);
        initVerb();
        setVerbNameString(%this.dialogList[%i]);
        setVerbXY(12, 145 + 11 * %d);
        setVerbColor($dialogColor);
        setVerbHiColor($dialogHiColor);
        setVerbOn();
        

        %v++; %d++;
    }

    // Turn off unused lines
    for ( ; %d < $MAX_DIALOG_LINES; %d++)
    {
        setCurrentVerb($dialogVerb0 + %d);
        setVerbOff();
        
    }

    echo("Sentence: " @ %firstSentence @ " " @ %lastSentence @ "}");
    echo("Shown: " @ %first @ " " @ %last @ "");

    // Up arrow
    setCurrentVerb(dialogUp);
    if (%first > %firstSentence)
    {
        initVerb();
        setVerbName("\x03");
        setVerbXY(2, 145);
        setVerbColor($dialogColor);
        setVerbHiColor($dialogHiColor);
        setVerbOn();
        
    }
    else
    {
        setVerbOff();
        
    }

    // Down arrow
    setCurrentVerb(dialogDown);
    if (%last < %lastSentence)
    {
        initVerb();
        setVerbName("\x02");
        setVerbXY(2, 145 + 4 * 11);
        setVerbColor($dialogColor);
        setVerbHiColor($dialogHiColor);
        setVerbOn();
        
    }
    else
    {
        setVerbOff();
        
    }

    // restore default charset
    initCharset(ResRoom.chtest);
}

// Handles mouse/keyboard over the dialog UI
function Dialog::dialogInputHandler(%this, %area, %cmd, %btn)
{
    %i = 0; %v = 0; %d = 0;

    echoBegin();
    echo("Area=" @ %area @ " cmd=" @ %cmd @ " button=" @ %btn @ "");
    echoEnd();

    egoPrintBegin();
    egoPrintOverhead();
    actorPrintEnd();

    // Area 4 is the keyboard
    if (%area == 4)
    {
        ResRoom.keyboardHandler(%cmd);
        return;
    }

    // Scroll arrows
    if (%cmd == dialogUp || %cmd == dialogDown)
    {
        %this.dialogOffset += ((%cmd == dialogUp) ? -1 : 1) * ($MAX_DIALOG_LINES / 2);

        if (%this.dialogOffset > %this.numActiveDialog - $MAX_DIALOG_LINES)
            %this.dialogOffset = %this.numActiveDialog - $MAX_DIALOG_LINES;

        if (%this.dialogOffset < 0)
            %this.dialogOffset = 0;

        echo("Dialog offset: %i{" @ %this.dialogOffset @ "}");
        Dialog.showDialog();
        return;
    }

    if (%cmd < $dialogVerb0 || %cmd > dialogVerb4)
        return;

    // Find which sentence was clicked
    %v = 0; %d = 0;
    for (%i = 0; %i < %this.numDialog; %i++)
    {
        if (%this.dialogList[%i] $= "") 
        {
            continue;
        }

        if (%v < %this.dialogOffset || %v >= %this.dialogOffset + $MAX_DIALOG_LINES)
        {
            %v++;
            continue;
        }

        if (%d == (%cmd - $dialogVerb0)) break;

        %v++; %d++;
    }

    %this.selectedSentence = %i;
}

// Begin a dialog UI session
function Dialog::dialogStart(%this, %color, %hiColor)
{
    %this.selectedSentence = -1;
    %this.dialogOffset = 0;

    if (%this.numDialog < 1)
    {
        echo("No dialog was setup, nothing to show.");
        %this.selectedSentence = -2;
        return;
    }

    // Hide normal verbs/inventory
    Verbs.showVerbs(0);

    // Configure dialog colors and draw UI
    $dialogColor   = (%color   ? %color   : $VERB_COLOR);
    $dialogHiColor = (%hiColor ? %hiColor : $VERB_HI_COLOR);
    %this.showDialog();

    // Stop mouseWatch while dialog is active
    if (isScriptRunning(ResRoom.mouseWatch))
        stopScript(ResRoom.mouseWatch);

    // Route input to our handler
    ResRoom.inputDelegate = ResRoom;
    ResRoom.inputHandler = dialogInputHandler;
}

// Hide dialog UI (but do not restore verb bar)
function Dialog::dialogHide(%this)
{
    %i = 0;

    for (%i = 0; %i < $MAX_DIALOG_LINES; %i++)
    {
        setCurrentVerb($dialogVerb0 + %i);
        setVerbOff();
        
    }

    setCurrentVerb(dialogUp);   setVerbOff(); 
    setCurrentVerb(dialogDown); setVerbOff(); 
}

// End dialog, restore verb bar + input, restart mouse watch
function Dialog::dialogEnd(%this)
{
    %this.dialogHide();
    Verbs.showVerbs(1);
    ResRoom.inputDelegate = ResRoom;
    ResRoom.inputHandler = defaultInputHandler;

    // Restart the mouse watching thread
    startRoomScript(ResRoom, mouseWatch);
}
