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

// ---------- Art & palette ----------
new Palette(zifPalette)
{
    source = "../graphics/rooms/back01_merged.bmp";
};

// ---------- ImageSets (pictures) ----------

new SimSet(zif_ImageSets)
{
    new ImageSet([walkingE])
    {
        format = "../graphics/zif/frames/walk_W_??.bmp";
        offset = "-13 -48";
        flags  = TRANSPARENT;
    };
    new ImageSet([walkingW])
    {
        format = "../graphics/zif/frames/walk_W_??.bmp";
        offset = "-13 -48";
        flags  = TRANSPARENT;
    };
    new ImageSet([walkingN])
    {
        format = "../graphics/zif/frames/walk_N_??.bmp";
        offset = "-13 -48";
        flags  = TRANSPARENT;
    };
    new ImageSet([walkingS])
    {
        format = "../graphics/zif/frames/walk_S_??.bmp";
        offset = "-13 -48";
        flags  = TRANSPARENT;
    };

    new ImageSet([standE]) { path = "../graphics/zif/frames/stand_SE.bmp";        offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([standW]) { path = "../graphics/zif/frames/stand_SE.bmp";        offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([standN]) { path = "../graphics/zif/frames/stand_N.bmp";         offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([standS]) { path = "../graphics/zif/frames/stand_S.bmp";         offset = "-13 -48"; flags = TRANSPARENT; };

    new ImageSet([standWithPhoneE]) { path = "../graphics/zif/frames/standwithphone_SE.bmp"; offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([standWithPhoneW]) { path = "../graphics/zif/frames/standwithphone_SE.bmp"; offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([standWithPhoneN]) { path = "../graphics/zif/frames/standwithphone_N.bmp";  offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([standWithPhoneS]) { path = "../graphics/zif/frames/standwithphone_S.bmp";  offset = "-13 -48"; flags = TRANSPARENT; };

    new ImageSet([talkingE]) { format = "../graphics/zif/frames/talk_SE_??.bmp";  offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([talkingW]) { format = "../graphics/zif/frames/talk_SE_??.bmp";  offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([talkingN]) { path   = "../graphics/zif/frames/talk_N_02.bmp";   offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([talkingS]) { format = "../graphics/zif/frames/talk_S_??.bmp";   offset = "-13 -48"; flags = TRANSPARENT; };

    new ImageSet([talkToPhoneE]) { format = "../graphics/zif/frames/talktophone_SE_??.bmp"; offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([talkToPhoneW]) { format = "../graphics/zif/frames/talktophone_SE_??.bmp"; offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([talkToPhoneN]) { path   = "../graphics/zif/frames/talktophone_N_02.bmp";  offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([talkToPhoneS]) { format = "../graphics/zif/frames/talktophone_S_??.bmp";  offset = "-13 -48"; flags = TRANSPARENT; };

    new ImageSet([raiseArmE])   { format = "../graphics/zif/frames/raisearm_SE_??.bmp";     offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([lookAroundS]) { format = "../graphics/zif/frames/lookaround_S_??.bmp";    offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([beamFx])      { format = "../graphics/zif/frames/beam_??.bmp";            offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([aimE])        { format = "../graphics/zif/frames/aim_E_??.bmp";           offset = "-13 -48"; flags = TRANSPARENT; };
    new ImageSet([aimUpE])      { path   = "../graphics/zif/frames/aimup_E.bmp";            offset = "-13 -49"; flags = TRANSPARENT; };
    new ImageSet([fireUpE])     { path   = "../graphics/zif/frames/fireup_E.bmp";           offset = "-13 -57"; flags = TRANSPARENT; };
    new ImageSet([fireStraightE]){ path  = "../graphics/zif/frames/firestraight_E.bmp";     offset = "-13 -48"; flags = TRANSPARENT; };
};

// ---------- Costume ----------
new Costume(ZifCostume)
{
    palette = zifPalette;
    flags   = FLIP;          // costume-wide
    limbs   = body;          // single limb

    // --- Anim: walk ---
    new CostumeAnim(walk)
    {
        E[body] = zif_ImageSets->walkingE.pick(0,3);
        W[body] = zif_ImageSets->walkingW.pick(0,3);
        S[body] = zif_ImageSets->walkingS.pick(0,3);
        N[body] = zif_ImageSets->walkingN.pick(0,3);
        // loops by default
    };

    // --- Anim: stand ---
    new CostumeAnim(stand)
    {
        E[body] = zif_ImageSets->standE.pick(0);
        W[body] = zif_ImageSets->standW.pick(0);
        N[body] = zif_ImageSets->standN.pick(0);
        S[body] = zif_ImageSets->standS.pick(0);
    };

    // --- Anim: init (same as stand) ---
    new CostumeAnim(init)
    {
        E[body] = zif_ImageSets->standE.pick(0);
        W[body] = zif_ImageSets->standW.pick(0);
        N[body] = zif_ImageSets->standN.pick(0);
        S[body] = zif_ImageSets->standS.pick(0);
    };

    // --- Anim: talkStart ---
    new CostumeAnim(talkStart)
    {
        E[body] = zif_ImageSets->talkingE.pick(0,2);   // 0000..0002
        W[body] = zif_ImageSets->talkingW.pick(0,2);
        S[body] = zif_ImageSets->talkingS.pick(0,2);
        N[body] = zif_ImageSets->standN.pick(0), talkingN.pick(0), standN.pick(0);
    };

    // --- Anim: talkStop ---
    new CostumeAnim(talkStop)
    {
        E[body] = zif_ImageSets->standE.pick(0);
        W[body] = zif_ImageSets->standW.pick(0);
        N[body] = zif_ImageSets->standN.pick(0);
        S[body] = zif_ImageSets->standS.pick(0);
    };

    // --- Anim: raiseArm  (@24, one-shot) ---
    new CostumeAnim(raiseArm)
    {
        id = 24;
        E[body] = zif_ImageSets->raiseArmE.pick(0,1);
        W[body] = zif_ImageSets->raiseArmE.pick(0,1);
        N[body] = zif_ImageSets->raiseArmE.pick(0,1);
        S[body] = zif_ImageSets->raiseArmE.pick(0,1);
        flags   = NO_LOOP;
    };

    // --- Anim: lowerArm (@28, one-shot) ---
    new CostumeAnim(lowerArm)
    {
        id = 28;
        E[body] = zif_ImageSets->raiseArmE.pick(0), standE.pick(0);
        W[body] = zif_ImageSets->raiseArmE.pick(0), standW.pick(0);
        N[body] = zif_ImageSets->raiseArmE.pick(0), standN.pick(0);
        S[body] = zif_ImageSets->raiseArmE.pick(0), standS.pick(0);
        flags   = NO_LOOP;
    };

    // --- Anim: standWithPhone (@32) ---
    new CostumeAnim(standWithPhone)
    {
        id = 32;
        E[body] = zif_ImageSets->standWithPhoneE.pick(0);
        W[body] = zif_ImageSets->standWithPhoneW.pick(0);
        N[body] = zif_ImageSets->standWithPhoneN.pick(0);
        S[body] = zif_ImageSets->standWithPhoneS.pick(0);
    };

    // --- Anim: talkToPhoneStart (@36) ---
    new CostumeAnim(talkToPhoneStart)
    {
        id = 36;
        E[body] = zif_ImageSets->talkToPhoneE.pick(0,2);
        W[body] = zif_ImageSets->talkToPhoneW.pick(0,2);
        S[body] = zif_ImageSets->talkToPhoneS.pick(0,2);
        N[body] = standWithPhoneN.pick(0), talkingN.pick(0), standWithPhoneN.pick(0) ;
    };

    // --- Anim: lookAround (@40) ---
    new CostumeAnim(lookAround)
    {
        id = 40;
        E[body] =  lookAroundS.pick(0), standWithPhoneS.pick(0), lookAroundS.pick(1), standWithPhoneE.pick(0);
        W[body] =  lookAroundS.pick(0), standWithPhoneS.pick(0), lookAroundS.pick(1), standWithPhoneW.pick(0);
        S[body] =  lookAroundS.pick(0), standWithPhoneS.pick(0), lookAroundS.pick(1), standWithPhoneS.pick(0);
        N[body] =  lookAroundS.pick(0), standWithPhoneS.pick(0), lookAroundS.pick(1), standWithPhoneN.pick(0);
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

    // --- Anim: fireup (@48, one-shot) ---
    new CostumeAnim(fireup)
    {
        id = 48;
        E[body] =  aimE.pick(0), aimE.pick(1), aimUpE.pick(0), fireUpE.pick(0), aimUpE.pick(0), aimE.pick(1), aimE.pick(0);
        W[body] =  aimE.pick(0), aimE.pick(1), aimUpE.pick(0), fireUpE.pick(0), aimUpE.pick(0), aimE.pick(1), aimE.pick(0);
        S[body] =  aimE.pick(0), aimE.pick(1), aimUpE.pick(0), fireUpE.pick(0), aimUpE.pick(0), aimE.pick(1), aimE.pick(0);
        N[body] =  aimE.pick(0), aimE.pick(1), aimUpE.pick(0), fireUpE.pick(0), aimUpE.pick(0), aimE.pick(1), aimE.pick(0);
        flags   = NO_LOOP;
    };

    // --- Anim: firestraight (@52, one-shot) ---
    new CostumeAnim(firestraight)
    {
        id = 52;
        E[body] =  aimE.pick(0), aimE.pick(1), aimE.pick(2), fireStraightE.pick(0), aimE.pick(2), aimE.pick(1), aimE.pick(0);
        W[body] =  aimE.pick(0), aimE.pick(1), aimE.pick(2), fireStraightE.pick(0), aimE.pick(2), aimE.pick(1), aimE.pick(0);
        S[body] =  aimE.pick(0), aimE.pick(1), aimE.pick(2), fireStraightE.pick(0), aimE.pick(2), aimE.pick(1), aimE.pick(0);
        N[body] =  aimE.pick(0), aimE.pick(1), aimE.pick(2), fireStraightE.pick(0), aimE.pick(2), aimE.pick(1), aimE.pick(0);
        flags   = NO_LOOP;
    };
};
