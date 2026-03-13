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


new SimSet(DialogVerbs)
{

};

// =========================
// Dialog namespace functions
// =========================

function Dialog::setupVerbs(%this)
{
    %verb = new VerbDisplay([upArrow]) {
        anchorPoint = 2, 145;
        displayText = "\x03";
    };
    %verb.setOn(false);
    DialogVerbs.add(%verb);

    %verb = new VerbDisplay([downArrow]) {
        anchorPoint = 145 + 4 * 11;
        displayText = "\x02";
    };
    %verb.setOn(false);
    DialogVerbs.add(%verb);
    
    for (%i=0; %i<$MAX_DIALOG_LINES; %i++)
    {
        %verb = new VerbDisplay([line @ %i]) {
            anchorPoint = 12, 145 + 11 * %i;
            line = %i;
            isLine = true;
        };
        %verb.setOn(false);
        DialogVerbs.add(%verb);
    }

    %this.numDialog = 0;
}

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

    echo("DIALOG STATE IS:");
    %this.dump();

    // Create lines
    %v = 0; %d = 0;
    for (%i = 0; %i < %this.numDialog; %i++)
    {
        if (%this.dialogList[%i] $= "")
        {
            continue;
        }

        if (%firstSentence < 0) 
        {
            %firstSentence = %i;
        }
        %lastSentence = %i;

        if (%v < %this.dialogOffset || %v >= %this.dialogOffset + $MAX_DIALOG_LINES)
        {
            %v++;
            continue;
        }

        if (%first < 0) 
        {
            %first = %i;
        }
        %last = %i;

        echo("GETTING VERB OBJECT:" @ %d+2 SPC "FOR INDEX:" @ %i);
        %verb = DialogVerbs.getObject(%d + 2);
        echo("TEXT SHOULD BE:" @ %this.dialogList[%i]);
        %verb.displayText = %this.dialogList[%i];
        %verb.setOn(true);

        %verb.color = $dialogColor;
        %verb.hiColor = $dialogHiColor;
        %verb.line = %i;

        %v++; 
        %d++;
    }

    // Turn off unused lines
    for ( ; %d < $MAX_DIALOG_LINES; %d++)
    {
        DialogVerbs.getObject(%d + 2).setOn(false);
    }

    echo("Sentence: " @ %firstSentence @ " " @ %lastSentence @ "}");
    echo("Shown: " @ %first @ " " @ %last @ "");

    // Up arrow
    %verb = DialogVerbs->upArrow;
    if (%first > %firstSentence)
    {
        %verb.setOn(true);
        %verb.color = $dialogColor;
        %verb.hiColor = $dialogHiColor;
    }
    else
    {
        %verb.setOn(false);
    }

    // Down arrow
    %verb = DialogVerbs->downArrow;
    if (%last < %lastSentence)
    {
        %verb.setOn(true);
        %verb.color = $dialogColor;
        %verb.hiColor = $dialogHiColor;
    }
    else
    {
        %verb.setOn(false);
    }
}

// Handles mouse/keyboard over the dialog UI
function Dialog::dialogInputHandler(%this, %area, %cmd, %btn)
{
    echo("Area=" @ %area @ " cmd=" @ %cmd @ " button=" @ %btn @ "");
    $selectedSentence = -1;

    // Area 4 is the keyboard
    if (%area == 4)
    {
        ResRoom.keyboardHandler(%cmd);
        return;
    }

    // Scroll arrows
    if (%cmd.getId() == DialogVerbs->upArrow.getId() || 
        %cmd.getId() == DialobVerbs->downArrow.getId())
    {
        %this.dialogOffset += ((%cmd.getId() == dialogUp.getId()) ? -1 : 1) * ($MAX_DIALOG_LINES / 2);

        if (%this.dialogOffset > %this.numActiveDialog - $MAX_DIALOG_LINES)
            %this.dialogOffset = %this.numActiveDialog - $MAX_DIALOG_LINES;

        if (%this.dialogOffset < 0)
            %this.dialogOffset = 0;

        echo("Dialog offset: %i{" @ %this.dialogOffset @ "}");
        %this.showDialog();
        return;
    }

    if (!%cmd.isLine)
    {
        return;
    }

    echo("SELECTED SENTENCE IS NOW:" @ $selectedSentence);
    $selectedSentence = %cmd.line;
}

// Begin a dialog UI session
function Dialog::dialogStart(%this, %color, %hiColor)
{
    echo("DIALOG START COLORS:" @ %color SPC "HI:" @ %hiColor);
    $selectedSentence = -1;
    %this.dialogOffset = 0;

    if (%this.numDialog < 1)
    {
        echo("No dialog was setup, nothing to show.");
        $selectedSentence = -2;
        return;
    }

    // Hide normal verbs/inventory
    Verbs.showVerbs(0);

    // Configure dialog colors and draw UI
    $dialogColor   = %color;
    $dialogHiColor = %hiColor;
    %this.showDialog();

    // Stop mouseWatch while dialog is active
    ResRoom.stopMouseWatch();

    // Route input to our handler
    ResRoom.inputDelegate = Dialog;
    ResRoom.realInputHandler = dialogInputHandler;
}

// Hide dialog UI (but do not restore verb bar)
function Dialog::dialogHide(%this)
{
    for (%i = 0; %i < $MAX_DIALOG_LINES; %i++)
    {
        %verb = DialogVerbs.getObject(%i + 2);
        %verb.setOn(false);
    }

    DialogVerbs->upArrow.setOn(false);
    DialogVerbs->downArrow.setOn(false);
}

// End dialog, restore verb bar + input, restart mouse watch
function Dialog::dialogEnd(%this)
{
    echo("DIALOG ENDED");
    %this.dialogHide();
    Verbs.showVerbs(1);

    ResRoom.inputDelegate = ResRoom;
    ResRoom.realInputHandler = defaultInputHandler;

    // Restart the mouse watching thread
    ResRoom.resetMouseWatch();
}
