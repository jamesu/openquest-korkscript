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
    source = "../../graphics/rooms/back01_merged.bmp";
};

// ---------- ImageSets (pictures) ----------

new SimSet(zif_ImageSets)
{
    new ImageSet([walkingE])
    {
        format = "../../graphics/zif/frames/walk_W_??.bmp";
        offset = "-13 -48";
        flags  = $TRANSPARENT;
    };
    new ImageSet([walkingW])
    {
        format = "../../graphics/zif/frames/walk_W_??.bmp";
        offset = "-13 -48";
        flags  = $TRANSPARENT;
    };
    new ImageSet([walkingN])
    {
        format = "../../graphics/zif/frames/walk_N_??.bmp";
        offset = "-13 -48";
        flags  = $TRANSPARENT;
    };
    new ImageSet([walkingS])
    {
        format = "../../graphics/zif/frames/walk_S_??.bmp";
        offset = "-13 -48";
        flags  = $TRANSPARENT;
    };

    new ImageSet([standE]) { format = "../../graphics/zif/frames/stand_SE.bmp";        offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([standW]) { format = "../../graphics/zif/frames/stand_SE.bmp";        offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([standN]) { format = "../../graphics/zif/frames/stand_N.bmp";         offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([standS]) { format = "../../graphics/zif/frames/stand_S.bmp";         offset = "-13 -48"; flags = $TRANSPARENT; };

    new ImageSet([standWithPhoneE]) { format = "../../graphics/zif/frames/standwithphone_SE.bmp"; offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([standWithPhoneW]) { format = "../../graphics/zif/frames/standwithphone_SE.bmp"; offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([standWithPhoneN]) { format = "../../graphics/zif/frames/standwithphone_N.bmp";  offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([standWithPhoneS]) { format = "../../graphics/zif/frames/standwithphone_S.bmp";  offset = "-13 -48"; flags = $TRANSPARENT; };

    new ImageSet([talkingE]) { format = "../../graphics/zif/frames/talk_SE_??.bmp";  offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([talkingW]) { format = "../../graphics/zif/frames/talk_SE_??.bmp";  offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([talkingN]) { format   = "../../graphics/zif/frames/talk_N_02.bmp";   offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([talkingS]) { format = "../../graphics/zif/frames/talk_S_??.bmp";   offset = "-13 -48"; flags = $TRANSPARENT; };

    new ImageSet([talkToPhoneE]) { format = "../../graphics/zif/frames/talktophone_SE_??.bmp"; offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([talkToPhoneW]) { format = "../../graphics/zif/frames/talktophone_SE_??.bmp"; offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([talkToPhoneN]) { format   = "../../graphics/zif/frames/talktophone_N_02.bmp";  offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([talkToPhoneS]) { format = "../../graphics/zif/frames/talktophone_S_??.bmp";  offset = "-13 -48"; flags = $TRANSPARENT; };

    new ImageSet([raiseArmE])   { format = "../../graphics/zif/frames/raisearm_SE_??.bmp";     offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([lookAroundS]) { format = "../../graphics/zif/frames/lookaround_S_??.bmp";    offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([beamFx])      { format = "../../graphics/zif/frames/beam_??.bmp";            offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([aimE])        { format = "../../graphics/zif/frames/aim_E_??.bmp";           offset = "-13 -48"; flags = $TRANSPARENT; };
    new ImageSet([aimUpE])      { format   = "../../graphics/zif/frames/aimup_E.bmp";            offset = "-13 -49"; flags = $TRANSPARENT; };
    new ImageSet([fireUpE])     { format   = "../../graphics/zif/frames/fireup_E.bmp";           offset = "-13 -57"; flags = $TRANSPARENT; };
    new ImageSet([fireStraightE]){ format  = "../../graphics/zif/frames/firestraight_E.bmp";     offset = "-13 -48"; flags = $TRANSPARENT; };
};

// ---------- Costume ----------
new Costume(ZifCostume)
{
    palette = zifPalette;
    flags   = $FLIP;         // costume-wide
    limbs   = body;          // single limb
    baseTalkPos = 0, -48;

    // --- Anim: walk ---
    new CostumeAnim([walk])
    {
        E[body] = zif_ImageSets->walkingE.pick(1,4);
        W[body] = zif_ImageSets->walkingW.pick(1,4);
        S[body] = zif_ImageSets->walkingS.pick(1,4);
        N[body] = zif_ImageSets->walkingN.pick(1,4);
        // loops by default
    };

    // --- Anim: stand ---
    new CostumeAnim([stand])
    {
        E[body] = zif_ImageSets->standE.pick(0);
        W[body] = zif_ImageSets->standW.pick(0);
        N[body] = zif_ImageSets->standN.pick(0);
        S[body] = zif_ImageSets->standS.pick(0);
    };

    // --- Anim: init (same as stand) ---
    new CostumeAnim([init])
    {
        E[body] = zif_ImageSets->standE.pick(0);
        W[body] = zif_ImageSets->standW.pick(0);
        N[body] = zif_ImageSets->standN.pick(0);
        S[body] = zif_ImageSets->standS.pick(0);
    };

    // --- Anim: talkStart ---
    new CostumeAnim([talkStart])
    {
        E[body] = zif_ImageSets->talkingE.pick(1,3);
        W[body] = zif_ImageSets->talkingW.pick(1,3);
        S[body] = zif_ImageSets->talkingS.pick(1,3);
        N[body] = zif_ImageSets->standN.pick(0), zif_ImageSets->talkingN.pick(0), zif_ImageSets->standN.pick(0);
    };

    // --- Anim: talkStop ---
    new CostumeAnim([talkStop])
    {
        E[body] = zif_ImageSets->standE.pick(0);
        W[body] = zif_ImageSets->standW.pick(0);
        N[body] = zif_ImageSets->standN.pick(0);
        S[body] = zif_ImageSets->standS.pick(0);
    };

    // --- Anim: raiseArm  (@24, one-shot) ---
    new CostumeAnim([raiseArm])
    {
        id = 24;
        E[body] = zif_ImageSets->raiseArmE.pick(1,2);
        W[body] = zif_ImageSets->raiseArmE.pick(1,2);
        N[body] = zif_ImageSets->raiseArmE.pick(1,2);
        S[body] = zif_ImageSets->raiseArmE.pick(1,2);
        flags   = $NO_LOOP;
    };

    // --- Anim: lowerArm (@28, one-shot) ---
    new CostumeAnim([lowerArm])
    {
        id = 28;
        E[body] = zif_ImageSets->raiseArmE.pick(1), zif_ImageSets->standE.pick(0);
        W[body] = zif_ImageSets->raiseArmE.pick(1), zif_ImageSets->standW.pick(0);
        N[body] = zif_ImageSets->raiseArmE.pick(1), zif_ImageSets->standN.pick(0);
        S[body] = zif_ImageSets->raiseArmE.pick(1), zif_ImageSets->standS.pick(0);
        flags   = $NO_LOOP;
    };

    // --- Anim: standWithPhone (@32) ---
    new CostumeAnim([standWithPhone])
    {
        id = 32;
        E[body] = zif_ImageSets->standWithPhoneE.pick(0);
        W[body] = zif_ImageSets->standWithPhoneW.pick(0);
        N[body] = zif_ImageSets->standWithPhoneN.pick(0);
        S[body] = zif_ImageSets->standWithPhoneS.pick(0);
    };

    // --- Anim: talkToPhoneStart (@36) ---
    new CostumeAnim([talkToPhoneStart])
    {
        id = 36;
        E[body] = zif_ImageSets->talkToPhoneE.pick(1,3);
        W[body] = zif_ImageSets->talkToPhoneW.pick(1,3);
        S[body] = zif_ImageSets->talkToPhoneS.pick(1,3);
        N[body] = zif_ImageSets->standWithPhoneN.pick(0), zif_ImageSets->talkingN.pick(0), zif_ImageSets->standWithPhoneN.pick(0) ;
    };

    // --- Anim: lookAround (@40) ---
    new CostumeAnim([lookAround])
    {
        id = 40;
        E[body] = zif_ImageSets->lookAroundS.pick(1), zif_ImageSets->standWithPhoneS.pick(0),zif_ImageSets->lookAroundS.pick(2), zif_ImageSets->standWithPhoneE.pick(0);
        W[body] = zif_ImageSets->lookAroundS.pick(1), zif_ImageSets->standWithPhoneS.pick(0),zif_ImageSets->lookAroundS.pick(2), zif_ImageSets->standWithPhoneW.pick(0);
        S[body] = zif_ImageSets->lookAroundS.pick(1), zif_ImageSets->standWithPhoneS.pick(0),zif_ImageSets->lookAroundS.pick(2), zif_ImageSets->standWithPhoneS.pick(0);
        N[body] = zif_ImageSets->lookAroundS.pick(1), zif_ImageSets->standWithPhoneS.pick(0),zif_ImageSets->lookAroundS.pick(2), zif_ImageSets->standWithPhoneN.pick(0);
    };

    // --- Anim: beam (@44, one-shot) ---
    new CostumeAnim([beam])
    {
        id = 44;
        E[body] = zif_ImageSets->beamFx.pick(1,11), zif_ImageSets->standE.pick(0);
        W[body] = zif_ImageSets->beamFx.pick(1,11), zif_ImageSets->standW.pick(0);
        S[body] = zif_ImageSets->beamFx.pick(1,11), zif_ImageSets->standS.pick(0);
        N[body] = zif_ImageSets->beamFx.pick(1,11), zif_ImageSets->standN.pick(0);
        flags   = $NO_LOOP;
    };

    // --- Anim: fireup (@48, one-shot) ---
    new CostumeAnim([fireup])
    {
        id = 48;
        E[body] =  zif_ImageSets->aimE.pick(1), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimUpE.pick(1), zif_ImageSets->fireUpE.pick(0), zif_ImageSets->aimUpE.pick(0), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimE.pick(1);
        W[body] =  zif_ImageSets->aimE.pick(1), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimUpE.pick(1), zif_ImageSets->fireUpE.pick(0), zif_ImageSets->aimUpE.pick(0), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimE.pick(1);
        S[body] =  zif_ImageSets->aimE.pick(1), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimUpE.pick(1), zif_ImageSets->fireUpE.pick(0), zif_ImageSets->aimUpE.pick(0), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimE.pick(1);
        N[body] =  zif_ImageSets->aimE.pick(1), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimUpE.pick(1), zif_ImageSets->fireUpE.pick(0), zif_ImageSets->aimUpE.pick(0), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimE.pick(1);
        flags   = $NO_LOOP;
    };

    // --- Anim: firestraight (@52, one-shot) ---
    new CostumeAnim([firestraight])
    {
        id = 52;
        E[body] =  zif_ImageSets->aimE.pick(1), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimE.pick(3), zif_ImageSets->fireStraightE.pick(0), zif_ImageSets->aimE.pick(3), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimE.pick(1);
        W[body] =  zif_ImageSets->aimE.pick(1), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimE.pick(3), zif_ImageSets->fireStraightE.pick(0), zif_ImageSets->aimE.pick(3), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimE.pick(1);
        S[body] =  zif_ImageSets->aimE.pick(1), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimE.pick(3), zif_ImageSets->fireStraightE.pick(0), zif_ImageSets->aimE.pick(3), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimE.pick(1);
        N[body] =  zif_ImageSets->aimE.pick(1), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimE.pick(3), zif_ImageSets->fireStraightE.pick(0), zif_ImageSets->aimE.pick(3), zif_ImageSets->aimE.pick(2), zif_ImageSets->aimE.pick(1);
        flags   = $NO_LOOP;
    };
};

ZifCostume.compileCostume();