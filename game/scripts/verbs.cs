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
// Resources / globals
// =========================

// “verb background” graphic
new ImageSet(VerbBackground_Set)
{
    path   = "graphics/verbs/verb_background.bmp";
    offset = "0 0";
    flags  = TRANSPARENT;
};

// Charset used by verbs
new Charset(verbChset)
{
    source = "verbs.char";
};

// Track whether the verb UI is “on”
$verbsOn = 0;

// Verbs in this case are globals

new RoomObject(VerbRootBg)
{
  anchorPoint = 0, 144;
  state = 1;
  
  new RoomObjectState()
  {
     image = "graphics/verbs/verb_background.bmp";
  };
};

$VBACK: TypeColor = 255, 255, 255, 0;
$VCOL: TypeColor = 255, 255, 255, 255;
$VHI: TypeColor = 255, 255, 0, 255;
$VDIM: TypeColor = 128, 128, 128, 255;

echo("VERBS LOADING");


function Verbs::onAdd(%this) 
{
    // Add inventory verbs
    %ic = 0;
    echo("Making", $INVENTORY_LINE SPC $INVENTORY_COL, "-verbs");
    for(%l = 0; %l < $INVENTORY_LINE ; %l++)
    {
        for(%c = 0 ; %c < $INVENTORY_COL ; %c++) {

            %px = 216 + %c*48;
            %py = 162 + %l*18;
            echo("MAKE: " @ inv @ %ic SPC %px SPC %py);
            echo($VCOL);

            nop();

            %verb = new VerbDisplay([inv @ %ic]) {
               displayText = "[I]";
               anchorPoint = %px, %py;
               backColor = $VBACK;
               color = $VCOL;
               hiColor = $VHI;
               dimColor = $VDIM;
               center = true;
               enabled = 1;
            };

            echo("POINT=" @ %verb.anchorPoint);

            echo(%verb.getInternalName());
            echo(%verb.getId());

            %this.add(%verb);

            %ic++;
        }
    }
}

// Verbs collection
new SimSet(Verbs)
{
    new VerbDisplay([backgroundVerb])
    {
       roomObject = VerbRootBg;
       anchorPoint = 0, 144;
    };

    new VerbDisplay([SntcLine]) {
    
       displayText = "%v{sntcVerb} %n{sntcObjADesc} %s{sntcPrepo} %n{sntcObjBDesc}";
       anchorPoint = 160, 147;
       color = $VDIM;
       center = 1;
    };

    new VerbDisplay([WalkTo]) {
    
       displayText = "Walk to";
       hotKey = "w";
    };
    
    new VerbDisplay([Give]) {
    
       displayText = "Give";
       anchorPoint = 146, 174;
       backColor = $VBACK;
       color = $VCOL;
       hiColor = $VHI;
       dimColor = $VDIM;
       center = 1;
       enabled = 1;
       hotKey = "g";
        isPreposition = true;
    };

    new VerbDisplay([PickUp]) {
    
       displayText = "Pick up";
       anchorPoint = 102, 161;
       backColor = $VBACK;
       color = $VCOL;
       hiColor = $VHI;
       dimColor = $VDIM;
       center = 1;
       enabled = 1;
       hotKey = "p";
        isPreposition = true;
    };

    new VerbDisplay([Use]) {
    
       displayText = "Use";
       anchorPoint = 146, 187;
       backColor = $VBACK;
       color = $VCOL;
       hiColor = $VHI;
       dimColor = $VDIM;
       center = 1;
       enabled = 1;
       hotKey = "u";
        isPreposition = true;
    };

    new VerbDisplay([Open]) {
    
       displayText = "Open";
       anchorPoint = 188, 161;
       backColor = $VBACK;
       color = $VCOL;
       hiColor = $VHI;
       dimColor = $VDIM;
       center = 1;
       enabled = 1;
       hotKey = "o";
        isPreposition = true;
    };

    new VerbDisplay([LookAt]) {
    
       displayText = "Examine";
       anchorPoint = 146, 161;
       backColor = $VBACK;
       color = $VCOL;
       hiColor = $VHI;
       dimColor = $VDIM;
       center = 1;
       enabled = 1;
       hotKey = "e";
        isPreposition = true;
    };

    new VerbDisplay([Smell]) {
    
       displayText = "Smell";
       anchorPoint = 188, 174;
       backColor = $VBACK;
       color = $VCOL;
       hiColor = $VHI;
       dimColor = $VDIM;
       center = 1;
       enabled = 1;
       hotKey = "s";
        isPreposition = true;
    };

    new VerbDisplay([TalkTo]) {
    
       displayText = "Talk to";
       anchorPoint = 102, 174;
       backColor = $VBACK;
       color = $VCOL;
       hiColor = $VHI;
       dimColor = $VDIM;
       center = 1;
       enabled = 1;
       hotKey = "t";
        isPreposition = true;
    };

    new VerbDisplay([Move]) {
    
       displayText = "Move";
       anchorPoint = 188, 187;
       backColor = $VBACK;
       color = $VCOL;
       hiColor = $VHI;
       dimColor = $VDIM;
       center = 1;
       enabled = 1;
       hotKey = "m";
        isPreposition = true;
    };

    // Inventory scrollers
    new VerbDisplay([invUp]) {
       displayText = "\x03";     // up arrow glyph
          anchorPoint = 309, 165;
       backColor = $VBACK;
       color = $VCOL;
       hiColor = $VHI;
       dimColor = $VDIM;
    };

    new VerbDisplay([invDown]) {
       displayText = "\x02";     // down arrow glyph
          anchorPoint = 309, 185;
       backColor = $VBACK;
       color = $VCOL;
       hiColor = $VHI;
       dimColor = $VDIM;
    };

    // NOTE: inventory items populated here
};

function Verbs::setupVerbs(%this)
{
    // Main verbs
    $sntcVerb = %this-->WalkTo;
    $sntcLine = %this-->SntcLine;

    for (%i=0; %i<8; %i++)
    {
        %this.invObj[%i] = 0;
    }
}


// Toggles the verb UI on/off and restores/saves groups.
function Verbs::showVerbs(%this, %show)
{
    if (%show == $verbsOn) 
    {
        return;
    }

    foreach(%verb in %this)
    {
        %verb.setOn(%show);
    }

    if (%show)
    {
        ResRoom.inventoryHandler(0);            // recalc inventory display
    }

    $verbsOn = %show;
}


function Verbs::setup(%this)
{
    %this.setupVerbs();
}
