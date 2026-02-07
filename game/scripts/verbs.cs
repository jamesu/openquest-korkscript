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

// =========================
// Room: Verbs
// =========================
new Room(Verbs)
{
    new RoomObject(background)
    {
        position    = 0, 144;                 // SCUMM puts the bar at y=144
        extent      = 0, 0;                   // optional if your UI sizes to image
        description = "Verb Background";
        imageSet    = VerbBackground_Set;
    };

    new VerbDisplay([backgroundVerb])
    {

    };

    new VerbDisplay([$SntcLine])
    {

    };

    new ContainerDisplay([mainVerbs])
    {
        new VerbDisplay([WalkTo])
        {

        };

        new VerbDisplay([Give])
        {

        };

        new VerbDisplay([PickUp])
        {

        };

        new VerbDisplay([Use])
        {

        };

        new VerbDisplay([Open])
        {

        };

        new VerbDisplay([LookAt])
        {

        };

        new VerbDisplay([Smell])
        {

        };

        new VerbDisplay([TalkTo])
        {

        };

        new VerbDisplay([Move])
        {

        };

        new VerbDisplay([invUp])
        {

        };

        new VerbDisplay([invDown])
        {

        };
    };

    new ContainerDisplay([invItems])
    {

    };
};


// =========================
// Verbs namespace functions
// =========================

// Sets up all verbs, labels, positions, colors, and inventory slots.
function Verbs::setupVerbs(%this)
{
    %color     = $VERB_COLOR;
    %hiColor   = $VERB_HI_COLOR;
    %dimColor  = $VERB_DIM_COLOR;
    %backColor = $VERB_BACK_COLOR;

    initCharset(verbChset);

    // Background “verb” occupies the art; anchor it.
    setCurrentVerb(backgroundVerb);
    initVerb();
    setVerbObject(background, Verbs);
    setVerbXY(0, 144);

    // Sentence line
    $sntcVerb = WalkTo;
    // sntcObjA / sntcObjB are set elsewhere as needed.

    setCurrentVerb($SntcLine);
    initVerb();
    setVerbName("%v{sntcVerb} %n{sntcObjADesc} %s{sntcPrepo} %n{sntcObjBDesc}");
    setVerbXY(160, 147);
    setVerbColor(%dimColor);
    verbCenter();

    // Main verbs
    setCurrentVerb(WalkTo);
    initVerb();
    setVerbName("Walk to");
    setVerbKey('w');

    setCurrentVerb(Give);
    initVerb();
    setVerbName("Give");
    setVerbXY(146, 174);
    setVerbBackColor(%backColor);
    setVerbColor(%color);
    setVerbHiColor(%hiColor);
    setVerbDimColor(%dimColor);
    verbCenter();
    setVerbOn();
    setVerbKey('g');

    setCurrentVerb(PickUp);
    initVerb();
    setVerbName("Pick up");
    setVerbXY(102, 161);
    setVerbBackColor(%backColor);
    setVerbColor(%color);
    setVerbHiColor(%hiColor);
    setVerbDimColor(%dimColor);
    verbCenter();
    setVerbOn();
    setVerbKey('p');

    setCurrentVerb(Use);
    initVerb();
    setVerbName("Use");
    setVerbXY(146, 187);
    setVerbBackColor(%backColor);
    setVerbColor(%color);
    setVerbHiColor(%hiColor);
    setVerbDimColor(%dimColor);
    verbCenter();
    setVerbOn();
    setVerbKey('u');

    setCurrentVerb(Open);
    initVerb();
    setVerbName("Open");
    setVerbXY(188, 161);
    setVerbBackColor(%backColor);
    setVerbColor(%color);
    setVerbHiColor(%hiColor);
    setVerbDimColor(%dimColor);
    verbCenter();
    setVerbOn();
    setVerbKey('o');

    setCurrentVerb(LookAt);
    initVerb();
    setVerbName("Examine");
    setVerbXY(146, 161);
    setVerbBackColor(%backColor);
    setVerbColor(%color);
    setVerbHiColor(%hiColor);
    setVerbDimColor(%dimColor);
    verbCenter();
    setVerbOn();
    setVerbKey('e');

    setCurrentVerb(Smell);
    initVerb();
    setVerbName("Smell");
    setVerbXY(188, 174);
    setVerbBackColor(%backColor);
    setVerbColor(%color);
    setVerbHiColor(%hiColor);
    setVerbDimColor(%dimColor);
    verbCenter();
    setVerbOn();
    setVerbKey('s');

    setCurrentVerb(TalkTo);
    initVerb();
    setVerbName("Talk to");
    setVerbXY(102, 174);
    setVerbBackColor(%backColor);
    setVerbColor(%color);
    setVerbHiColor(%hiColor);
    setVerbDimColor(%dimColor);
    verbCenter();
    setVerbOn();
    setVerbKey('t');

    setCurrentVerb(Move);
    initVerb();
    setVerbName("Move");
    setVerbXY(188, 187);
    setVerbBackColor(%backColor);
    setVerbColor(%color);
    setVerbHiColor(%hiColor);
    setVerbDimColor(%dimColor);
    verbCenter();
    setVerbOn();
    setVerbKey('m');

    // Inventory scrollers
    setCurrentVerb(invUp);
    initVerb();
    setVerbName("\x03");     // up arrow glyph
    setVerbXY(309, 165);
    setVerbBackColor(%backColor);
    setVerbColor(%color);
    setVerbHiColor(%hiColor);
    setVerbDimColor(%dimColor);

    setCurrentVerb(invDown);
    initVerb();
    setVerbName("\x02");     // down arrow glyph
    setVerbXY(309, 185);
    setVerbBackColor(%backColor);
    setVerbColor(%color);
    setVerbHiColor(%hiColor);
    setVerbDimColor(%dimColor);

    // Inventory grid (visibility managed by save/restore below)
    %vrb = 0;
    for (%l = 0; %l < $INVENTORY_LINE; %l++)
    {
        for (%c = 0; %c < $INVENTORY_COL; %c++)
        {
            %vrb = new VerbDisplay() {
                internalName = invSlot @ %i;
            };
            %vrb.init();
            %vrb.setOn();
            %vrb.setPos(216 + %c*48, 162 + %l*18);
            %this->invItems.add(%vrb);
            %vrb++;
        }
    }

    for (%i=0; %i<8; %i++)
    {
        %this.invObj[%i] = 0;
    }
}


// Toggles the verb UI on/off and restores/saves groups.
function Verbs::showVerbs(%this, %show)
{
    if (%show == $verbsOn) return;

    foreach(%verb in %this->mainVerbs)
    {
        %verb.setOn(%show);
    }

    foreach(%verb in %this->invItems)
    {
        %verb.setOn(%show);
    }

    %this->SntcLine->setOn(%show);
    %this->invUp->setOn(%show);
    %this->invDown->setOn(%show);
    %this->backgroundVerb->setOn(%show);

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
