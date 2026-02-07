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

// Derived from zif.cs

// ---------- Art & palette ----------
new Palette(zobPalette)
{
    source = "../graphics/rooms/back01_merged.bmp";
};

// ---------- ImageSets (pictures) ----------

new SimSet(zob_ImageSets)
{
    new ImageSet([walkingE])
    {
        format = "../graphics/zob/frames/walk_E_??.bmp";
        offset = "-13 -42";
        flags  = TRANSPARENT;
    };
    new ImageSet([walkingW])
    {
        format = "../graphics/zob/frames/walk_E_??.bmp";
        offset = "-13 -42";
        flags  = TRANSPARENT;
    };
    new ImageSet([walkingN])
    {
        format = "../graphics/zob/frames/walk_N_??.bmp";
        offset = "-13 -42";
        flags  = TRANSPARENT;
    };
    new ImageSet([walkingS])
    {
        format = "../graphics/zob/frames/walk_S_??.bmp";
        offset = "-13 -42";
        flags  = TRANSPARENT;
    };

    new ImageSet([standE]) { path = "../graphics/zob/frames/stand_SE.bmp";        offset = "-13 -42"; flags = TRANSPARENT; };
    new ImageSet([standW]) { path = "../graphics/zob/frames/stand_SE.bmp";        offset = "-13 -42"; flags = TRANSPARENT; };
    new ImageSet([standN]) { path = "../graphics/zob/frames/stand_N.bmp";         offset = "-13 -42"; flags = TRANSPARENT; };
    new ImageSet([standS]) { path = "../graphics/zob/frames/stand_S.bmp";         offset = "-13 -42"; flags = TRANSPARENT; };

    new ImageSet([talkingE]) { format = "../graphics/zob/frames/talk_SE_??.bmp";  offset = "-13 -42"; flags = TRANSPARENT; };
    new ImageSet([talkingW]) { format = "../graphics/zob/frames/talk_SE_??.bmp";  offset = "-13 -42"; flags = TRANSPARENT; };
    new ImageSet([talkingN]) { path   = "../graphics/zob/frames/talk_N_02.bmp";   offset = "-13 -42"; flags = TRANSPARENT; };
    new ImageSet([talkingS]) { format = "../graphics/zob/frames/talk_S_??.bmp";   offset = "-13 -42"; flags = TRANSPARENT; };

    new ImageSet([raiseArmE])   { format = "../graphics/zob/frames/raisearm_SE_??.bmp";     offset = "-13 -42"; flags = TRANSPARENT; };
    new ImageSet([raiseArmN])   { format = "../graphics/zob/frames/raisearm_N_??.bmp";     offset = "-13 -42"; flags = TRANSPARENT; };
    new ImageSet([scanS])       { format = "../graphics/zob/frames/scan_S_??.bmp";    offset = "-13 -42"; flags = TRANSPARENT; };
    new ImageSet([aimE])        { format = "../graphics/zob/frames/aim_E_??.bmp";           offset = "-13 -42"; flags = TRANSPARENT; };
    new ImageSet([fireUpE])     { path   = "../graphics/zob/frames/fireup_E.bmp";           offset = "-13 -57"; flags = TRANSPARENT; };
    new ImageSet([beam])        { path   = "../graphics/zob/frames/beam_??.bmp";           offset = "-13 -57"; flags = TRANSPARENT; };
    new ImageSet([die])         { path  = "../graphics/zob/frames/die_SE.bmp";     offset = "-13 -42"; flags = TRANSPARENT; };
};

// ---------- Costume ----------
new Costume(ZobCostume)
{
    palette = zobPalette;
    flags   = FLIP;             // costume-wide
    limbs   = body, talk_body;  // two limbs

    // --- Anim: walk ---
    new CostumeAnim(walk)
    {
        E[body] = zob_ImageSets->walkingE.pick(0,3);
        W[body] = zob_ImageSets->walkingW.pick(0,3);
        S[body] = zob_ImageSets->walkingS.pick(0,3);
        N[body] = zob_ImageSets->walkingN.pick(0,3);
        E[talk_body] = HIDE;
        W[talk_body] = HIDE;
        S[talk_body] = HIDE;
        N[talk_body] = HIDE;
        // loops by default
    };

    // --- Anim: stand ---
    new CostumeAnim(stand)
    {
        E[body] = zob_ImageSets->standE.pick(0);
        W[body] = zob_ImageSets->standW.pick(0);
        N[body] = zob_ImageSets->standN.pick(0);
        S[body] = zob_ImageSets->standS.pick(0);
        E[talk_body] = SHOW;
        W[talk_body] = SHOW;
        S[talk_body] = SHOW;
        N[talk_body] = SHOW;
    };

    // --- Anim: init (same as stand) ---
    new CostumeAnim(init)
    {
        E[body] = zob_ImageSets->standE.pick(0);
        W[body] = zob_ImageSets->standW.pick(0);
        N[body] = zob_ImageSets->standN.pick(0);
        S[body] = zob_ImageSets->standS.pick(0);
    };

    // --- Anim: talkStart ---
    new CostumeAnim(talkStart)
    {
        E[talk_body] = zob_ImageSets->talkingE.pick(0,2);   // 0000..0002
        W[talk_body] = zob_ImageSets->talkingW.pick(0,2);
        S[talk_body] = zob_ImageSets->talkingS.pick(0,2);
        N[talk_body] = zob_ImageSets->standN.pick(0), talkingN.pick(0), standN.pick(0);
    };

    // --- Anim: talkStop ---
    new CostumeAnim(talkStop)
    {
        E[talk_body] = HIDE;
        W[talk_body] = HIDE;
        S[talk_body] = HIDE;
        N[talk_body] = HIDE;
    };

    // --- Anim: raiseArm  (@24, one-shot) ---
    new CostumeAnim(raiseArm)
    {
        id = 24;
        E[body] = zob_ImageSets->raiseArmE.pick(0,1);
        W[body] = zob_ImageSets->raiseArmE.pick(0,1);
        N[body] = zob_ImageSets->raiseArmE.pick(0,1);
        S[body] = zob_ImageSets->raiseArmE.pick(0,1);
        flags   = NO_LOOP;
    };

    // --- Anim: lowerArm (@28, one-shot) ---
    new CostumeAnim(lowerArm)
    {
        id = 28;
        E[body] = zob_ImageSets->raiseArmE.pick(0), standE.pick(0);
        W[body] = zob_ImageSets->raiseArmE.pick(0), standW.pick(0);
        N[body] = zob_ImageSets->raiseArmE.pick(0), standN.pick(0);
        S[body] = zob_ImageSets->raiseArmE.pick(0), standS.pick(0);
        flags   = NO_LOOP;
    };

    // --- Anim: scan (@32) ---
    new CostumeAnim(scan)
    {
        id = 32;
        E[body] = zob_ImageSets->standWithPhoneE.pick(0);
        W[body] = zob_ImageSets->standWithPhoneW.pick(0);
        N[body] = zob_ImageSets->standWithPhoneN.pick(0);
        S[body] = zob_ImageSets->standWithPhoneS.pick(0);
    };

    // --- Anim: fireup (@40, one-shot) ---
    new CostumeAnim(fireup)
    {
        id = 40;
        E[body] =  aimE.pick(0), aimE.pick(1), aimUpE.pick(0), fireUpE.pick(0), aimUpE.pick(0), aimE.pick(1), aimE.pick(0);
        W[body] =  aimE.pick(0), aimE.pick(1), aimUpE.pick(0), fireUpE.pick(0), aimUpE.pick(0), aimE.pick(1), aimE.pick(0);
        S[body] =  aimE.pick(0), aimE.pick(1), aimUpE.pick(0), fireUpE.pick(0), aimUpE.pick(0), aimE.pick(1), aimE.pick(0);
        N[body] =  aimE.pick(0), aimE.pick(1), aimUpE.pick(0), fireUpE.pick(0), aimUpE.pick(0), aimE.pick(1), aimE.pick(0);
        flags   = NO_LOOP;
    };

    // --- Anim: beam (@44, one-shot) ---
    new CostumeAnim(beam)
    {
        id = 44;
        E[body] = beamFx.pick(0,10), standE.pick(0);
        W[body] = beamFx.pick(0,10), standW.pick(0);
        S[body] = beamFx.pick(0,10), standS.pick(0);
        N[body] = beamFx.pick(0,10), standN.pick(0);
        flags   = NO_LOOP;
    };

    // --- Anim: firestraight (@52, one-shot) ---
    new CostumeAnim(die)
    {
        id = 52;
        E[body] =  aimE.pick(0), aimE.pick(1), aimE.pick(2), fireStraightE.pick(0), aimE.pick(2), aimE.pick(1), aimE.pick(0);
        W[body] =  aimE.pick(0), aimE.pick(1), aimE.pick(2), fireStraightE.pick(0), aimE.pick(2), aimE.pick(1), aimE.pick(0);
        S[body] =  aimE.pick(0), aimE.pick(1), aimE.pick(2), fireStraightE.pick(0), aimE.pick(2), aimE.pick(1), aimE.pick(0);
        N[body] =  aimE.pick(0), aimE.pick(1), aimE.pick(2), fireStraightE.pick(0), aimE.pick(2), aimE.pick(1), aimE.pick(0);
        flags   = NO_LOOP;
    };
};
