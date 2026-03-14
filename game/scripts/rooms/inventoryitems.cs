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


// =========================
// Room: InventoryItems
// (NOTE: both objects & actors are displayables; here we only need objects.)
// =========================
new Room(InventoryItems)
{
    new RoomObject(InvAxe)
    {
        displayText = "the axe";
        state       = 0;
        
        new RoomObjectState()
        {
           image = "graphics/inventory_items/inv_axe.bmp";
         };
    };

    new RoomObject(InvGun)
    {
        displayText = "gun";
        state       = 1;
        
        new RoomObjectState()
        {
           image = "graphics/inventory_items/gun_40.bmp";
         };
    };

    new RoomObject(InvBullets)
    {
        displayText = "ammunition";
        state       = 1;
        
        new RoomObjectState()
        {
           image = "graphics/inventory_items/bullets.bmp";
         };
    };

    new RoomObject(InvCard)
    {
        displayText = "card";
        state       = 1;
        
        new RoomObjectState()
        {
           image = "graphics/inventory_items/card.bmp";
         };
    };

    new RoomObject(InvBatteries)
    {
        displayText = "batteries";
        state       = 1;
        
        new RoomObjectState()
        {
           image = "graphics/inventory_items/scanner_40.bmp";
         };
    };

    new RoomObject(InvScanner)
    {
        displayText = "scanner";
        state       = 1;
        
        new RoomObjectState()
        {
           image = "graphics/inventory_items/scanner_40.bmp";
         };
        new RoomObjectState()
        {
           image = "graphics/inventory_items/scanner_dead_40.bmp";
         };
    };
};


// =========================
// Helper / utility (no waits)
// =========================


function InventoryItems::loadGun(%this)
{
    gun.state = 2;
    bullets.owner.removeInventory(bullets);
    loadingGunSnd.play();
    egoSay("The weapon has been loaded.");
}

// Verb handlers for items

// -------- gun --------
// Some branches may call waitForMessage -> script.
function InvGun::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("A crude ballistic firearm fashioned from a primitive alloy.");
}

function InvGun::getPreposition(%this, %verb)
{
    if (%verb.internalName $= "Give")
        return "to";
    else
        return "on";
}

function InvGun::onUse(%this, %verb, %objA, %objB)
{
    if (!isObject(%objB))
    {
        %this.onDefaultAction(%verb, %objA, %objB);
        return;
    }

    switch$ (%objB.getName())
    {
        case "carol":
            egoSay("Terminating this lifeform could attract unwarranted attention.");
            if (gun.state < 2)
            {
                waitForMessage();
                egoSay("And the weapon is not loaded anyway.");
            }
            break;

        default:
            %this.onDefaultAction(%verb, %objA, %objB);
    }
}

function InvGun::onGive(%this, %verb, %objA, %objB)
{
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

// -------- bullets --------
function InvBullets::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("This ammunition may be compatible with a projectile weapon.");
}

function InvBullets::getPreposition(%this, %verb, %objA, %objB)
{
    if (%verb.internalName $= "Give")
        return "to";
    else
        return "with";
}

function InvBullets::onUsedWith(%this, %verb, %objA, %objB)
{
    %this.onUse(%verb, %objA, %objB);
}

function InvBullets::onUse(%this, %verb, %objA, %objB)
{
    if (!isObject(%objB))
    {
        %this.onDefaultAction(%verb, %objA, %objB);
        return;
    }

    switch$ (%objB.getName())
    {
        case "InvGun":
            %this.loadGun();
            break;

        default:
            %this.onDefaultAction(%verb, %objA, %objB);
    }
}

// -------- card --------
function InvCard::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("This device could be used to interact with some form of hardware.");
}

function InvCard::getPreposition(%this, %verb)
{
    if (%verb.internalName $= "Give")
        return "to";
    else
        return "on";
}

function InvCard::onUse(%this, %verb, %objA, %objB)
{
    if (!isObject(%objB))
    {
        %this.onDefaultAction(%verb, %objA, %objB);
        return;
    }

    switch$ (%objB.getName())
    {
        case "InvScanner":
            egoSay("The key isn't compatible with this model.");
            break;

        default:
            %this.onDefaultAction(%verb, %objA, %objB);
            break;
    }
}

function InvCard::onGive(%this, %verb, %objA, %objB)
{
    if (!isObject(%objB))
    {
        %this.onDefaultAction(%verb, %objA, %objB);
        return;
    }

    switch$ (%objB.getName())
    {
        case "carol":
            carol.say(
                "I see you've got mainframe clearance.\n" @
                "I'm gonna be dusting in there soon.\n" @
                "Try and keep it tidy.");
            break;

        case "commanderZif":
            Actors.pauseRoaming(commanderZif);
            commanderZif.say(
                "Excellent work Zob, a key.\n" @
                "Keep this, use it to infiltrate the mainframe.");
            waitForMessage();
            Actors.resumeRoaming(commanderZif);
            break;

        default:
            %this.onDefaultAction(%verb, %objA, %objB);
            break;
    }
}

// -------- batteries --------
function InvBatteries::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("They used to power the hand scanner.");
}

function InvBatteries::getPreposition(%this, %verb)
{
    if (%verb.internalName $= "Give")
        return "to";
    else
        return "with";
}

function InvBatteries::onUse(%this, %verb, %objA, %objB)
{
    switch$ (%objB.getName())
    {
        case "InvScanner":
            egoSay("They won't go back in.");
            break;

        default:
            %this.onDefaultAction(%verb, %objA, %objB);
            break;
    }
}

// -------- scanner --------
function InvScanner::onLookAt(%this, %verb, %objA, %objB)
{
    egoSay("Standard issue hand scanner.");
    waitForMessage();
    egoSay("Detects most of the EM spectrum.");
}

function InvScanner::onUse(%this, %verb, %objA, %objB)
{
    if (scanner.state == 2)
    {
        egoSay("The scanner no longer functions.");
        return;
    }
    egoSay("I'll scan the area.");
    waitForMessage();

    $VAR_EGO.animate(scan); delayFiber(30);
    $VAR_EGO.animate(scan); delayFiber(30);
    $VAR_EGO.animate(scan); delayFiber(30);

    if ($VAR_EGO.getGroup() == SecretRoom.getId())
        egoSay("The artifact is in this room.");
    else
        egoSay("I am detecting the artifact near this location.");
    return;
}

function InvScanner::getPreposition(%this, %verb)
{
    if (%verb.internalName $= "Give")
    {
        return "to";
    }
    else
    {
        return "";
    }
}

function InvScanner::onGive(%this, %verb, %objA, %objB)
{
    if (!isObject(%objB))
    {
        %this.onDefaultAction(%verb, %objA, %objB);
        return;
    }

    switch$ (%objB.getName())
    {
        case "commanderZif":
            Actors.pauseRoaming(commanderZif);
            commanderZif.say("You're the scientist on this team, you use it.");
            waitForMessage();
            Actors.resumeRoaming(commanderZif);
            break;

        default:
            %this.onDefaultAction(%verb, %objA, %objB);
    }
    return;
}

function InvScanner::isOpenable(%this)
{
    return true;
}

function InvScanner::onOpen(%this, %verb, %objA, %objB)
{
    if (scanner.state == 2)
    {
        egoSay("The casing appears to be broken.");
        return;
    }
    egoSay("I'll open up the housing.");
    waitForMessage();

    // Drop batteries into inventory
    $VAR_EGO.pickupObject(InvBatteries);

    %objA.state = 2;
    egoSay("The batteries have fallen out.");
    return;
}

