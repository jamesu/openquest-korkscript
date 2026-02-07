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
// Resources
// =========================

new Sound(loadingGunSnd) { path = "loading_gun.soun"; };

new ImageSet(Inv_Axe)     { path = "graphics/inventory_items/inv_axe.bmp";          flags = TRANSPARENT; };
new ImageSet(Inv_Gun)     { path = "graphics/inventory_items/gun_40.bmp";           flags = TRANSPARENT; };
new ImageSet(Inv_Bullets) { path = "graphics/inventory_items/bullets.bmp";          flags = TRANSPARENT; };
new ImageSet(Inv_Card)    { path = "graphics/inventory_items/card.bmp";             flags = TRANSPARENT; };
new ImageSet(Inv_Batts)   { path = "graphics/inventory_items/batteries.bmp";        flags = TRANSPARENT; };

// Scanner has two frames/states
new ImageSet(Inv_Scanner_0) { path = "graphics/inventory_items/scanner_40.bmp";     flags = TRANSPARENT; };
new ImageSet(Inv_Scanner_1) { path = "graphics/inventory_items/scanner_dead_40.bmp";flags = TRANSPARENT; };


// =========================
// Room: InventoryItems
// (NOTE: both objects & actors are displayables; here we only need objects.)
// =========================
new Room(InventoryItems)
{
    // Dimensions x/w/h present in SCUMMC source are kept where provided.

    new RoomObject(axe)
    {
        position    = 0, 0;     // x given in source, y not specified
        extent      = 40, 16;   // w,h from source
        description = "the axe";
        imageSet    = Inv_Axe;
        state       = 1;        // matches SCUMMC 'state = 1;'
    };

    new RoomObject(gun)
    {
        description = "gun";
        imageSet    = Inv_Gun;
        state       = 0;
    };

    new RoomObject(bullets)
    {
        description = "ammunition";
        imageSet    = Inv_Bullets;
    };

    new RoomObject(card)
    {
        description = "card";
        imageSet    = Inv_Card;
    };

    new RoomObject(batteries)
    {
        description = "batteries";
        imageSet    = Inv_Batts;
    };

    new RoomObject(scanner)
    {
        position    = 0, 0;     // x present in source; keep default
        extent      = 40, 16;   // w,h from source
        description = "scanner";
        imageSet    = Inv_Scanner_0;  // start on state 1 in SCUMMC; engine will reflect actual state via drawObject()
        state       = 1;
    };
};


// =========================
// Helper / utility (no waits)
// =========================

// Separate because setObjectOwner() can kill an object's script in SCUMM.
// No waits here -> function.
function InventoryItems::loadGun()
{
    setObjectState(gun, 2);
    setObjectOwner(bullets, 0);
    startSound(loadingGunSnd);
    egoSay("The weapon has been loaded.");
}


// =========================
// Verb handlers (namespace-bound to object names)
// Use `script` if any branch may call wait/delay.
// =========================

// -------- axe --------
function axe::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "Icon":
            // Start an object-specific script; preserve SCUMM call shape.
            //startScript2(%verb, [ InventoryItems::axe, %objA, %objB ]); // TOFIX
            return;
    }
}

// -------- gun --------
// Some branches may call waitForMessage -> script.
function gun::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            egoSay("A crude ballistic firearm fashioned from a primitive alloy.");
            return;

        case "Preposition":
            if (%verb $= "Give")
                $sntcPrepo[0] = "to";
            else
                $sntcPrepo[0] = "on";
            return;

        case "Use":
            switch$ (%objB)
            {
                case "carol":
                    egoSay("Terminating this lifeform could attract unwarranted attention.");
                    if (getObjectState(gun) < 2)
                    {
                        waitForMessage();
                        egoSay("And the weapon is not loaded anyway.");
                    }
                    break;

                default:
                    ResRoom::defaultAction(%verb, %objA, %objB);
            }
            return;

        case "Give":
            switch$ (%objB)
            {
                case "commanderZif":
                    if (!SecretRoom.hasShotAtNode)
                    {
                        egoSay("And lose my advantage?");
                        return;
                    }
                    SecretRoom.outro();
                    break;

                default:
                    egoSay("I might need this.");
            }
            return;
    }
}

// -------- bullets --------
function bullets::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            egoSay("This ammunition may be compatible with a projectile weapon.");
            return;

        case "Preposition":
            if (%verb $= "Give")
                $sntcPrepo[0] = "to";
            else
                $sntcPrepo[0] = "with";
            return;

        case "UsedWith":
        case "Use":
            switch$ (%objB)
            {
                case "gun":
                    %this.loadGun();
                    break;

                default:
                    ResRoom::defaultAction(%verb, %objA, %objB);
            }
            return;
    }
}

// -------- card --------
// Give->commanderZif branch uses wait -> script.
function card::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            egoSay("This device could be used to interact with some form of hardware.");
            return;

        case "Preposition":
            if (%verb $= "Give")
                $sntcPrepo[0] = "to";
            else
                $sntcPrepo[0] = "on";
            return;

        case "Use":
            switch$ (%objB)
            {
                case "scanner":
                    egoSay("The key isn't compatible with this model.");
                    break;

                default:
                    ResRoom::defaultAction(%verb, %objA, %objB);
                    break;
            }
            return;

        case "Give":
            switch$ (%objB)
            {
                case "carol":
                    actorSay(carol,
                        "I see you've got mainframe clearance.\w" @
                        "I'm gonna be dusting in there soon.\w" @
                        "Try and keep it tidy.");
                    break;

                case "commanderZif":
                    Actors::pauseRoaming(commanderZif);
                    actorSay(commanderZif,
                        "Excellent work Zob, a key.\w" @
                        "Keep this, use it to infiltrate the mainframe.");
                    waitForMessage();
                    Actors::resumeRoaming(commanderZif);
                    break;

                default:
                    ResRoom::defaultAction(%verb, %objA, %objB);
                    break;
            }
            return;
    }
}

// -------- batteries --------
function batteries::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            egoSay("They used to power the hand scanner.");
            return;

        case "Preposition":
            if (%verb $= "Give")
                $sntcPrepo[0] = "to";
            else
                $sntcPrepo[0] = "with";
            return;

        case "Use":
            switch$ (%objB)
            {
                case "scanner":
                    egoSay("They won't go back in.");
                    break;

                default:
                    ResRoom::defaultAction(%verb, %objA, %objB);
                    break;
            }
            return;
    }
}

// -------- scanner --------
// Multiple waits/delays -> script.
function scanner::onVerb(%this, %verb, %objA, %objB)
{
    switch$ (%verb)
    {
        case "LookAt":
            egoSay("Standard issue hand scanner.");
            waitForMessage();
            egoSay("Detects most of the EM spectrum.");
            return;

        case "Use":
            if (getObjectState(scanner) == 2)
            {
                egoSay("The scanner no longer functions.");
                return;
            }
            egoSay("I'll scan the area.");
            waitForMessage();

            animateActor(VAR_EGO, zob_anim_scan); delay(30);
            animateActor(VAR_EGO, zob_anim_scan); delay(30);
            animateActor(VAR_EGO, zob_anim_scan); delay(30);

            if (getActorRoom(VAR_EGO) == SecretRoom)
                egoSay("The artifact is in this room.");
            else
                egoSay("I am detecting the artifact near this location.");
            return;

        case "Preposition":
            if (%verb $= "Give")
                $sntcPrepo[0] = "to";
            return;

        case "Give":
            switch$ (%objB)
            {
                case "commanderZif":
                    Actors::pauseRoaming(commanderZif);
                    actorSay(commanderZif,"You're the scientist on this team, you use it.");
                    waitForMessage();
                    Actors::resumeRoaming(commanderZif);
                    break;

                default:
                    ResRoom::defaultAction(%verb, %objA, %objB);
            }
            return;

        case "Open":
            if (getObjectState(scanner) == 2)
            {
                egoSay("The casing appears to be broken.");
                return;
            }
            egoSay("I'll open up the housing.");
            waitForMessage();

            // Drop batteries into inventory
            pickupObject(batteries, InventoryItems);

            setObjectState(%objA, 2);
            drawObject(%objA, 2);
            egoSay("The batteries have fallen out.");
            return;
    }
}
