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

new Room(Skyline)
{
    image = "graphics/rooms/skyline.bmp";

    new Actor([ufoActor])
    {
        className = ZifClass;
    };
};

function Skyline::entry(%this)
{
    try 
    {
        beginCutscene();
        %actor = %this->ufoActor;
        %actor.init();
        %actor.setCostume(ufoCostume);
        %actor.setCostume(ufoCost);
        %actor.setTalkPos(-60,-70);
        %actor.setName("Ufo");
        %actor.setWalkSpeed(2,1);
        %actor.setTalkColor(ZIF_COLOR);
        %actor.setWidth(20);
        %actor.setIgnoreBoxes();
        %actor.setAnimSpeed(4);
        
        %actor.putAt(-50,80,Skyline);
        %actor.walkTo(140, 80);
        %actor.wait();
        %actor.say("These are the coordinates.");
        %actor.wait();
        %actor.say("Ensign, bring the teleportation device online.");
        %actor.wait();

        delay(100);
        endCutscene();

        // Won't get $CUTSCENE_OVERRIDE after this point
        screenEffect(0x8686);
    }
    catch ($CUTSCENE_OVERRIDE) 
    {
        %didSkip = true;
        stopTalking();
        screenEffect(0x8186);
    }
    
    startRoom(OfficeRoom);
}
