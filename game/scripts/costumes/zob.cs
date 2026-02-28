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
    source = "../../graphics/rooms/back01_merged.bmp";
};

// ---------- ImageSets (pictures) ----------

new SimSet(zob_ImageSets)
{
    new ImageSet([walkingE])
    {
        format = "../../graphics/zob/frames/walk_E_??.bmp";
        offset = "-13 -42";
        flags  = $TRANSPARENT;
    };
    new ImageSet([walkingW])
    {
        format = "../../graphics/zob/frames/walk_E_??.bmp";
        offset = "-13 -42";
        flags  = $TRANSPARENT;
    };
    new ImageSet([walkingN])
    {
        format = "../../graphics/zob/frames/walk_N_??.bmp";
        offset = "-13 -42";
        flags  = $TRANSPARENT;
    };
    new ImageSet([walkingS])
    {
        format = "../../graphics/zob/frames/walk_S_??.bmp";
        offset = "-13 -42";
        flags  = $TRANSPARENT;
    };

    new ImageSet([standE]) { format = "../../graphics/zob/frames/stand_SE.bmp";        offset = "-13 -42"; flags = $TRANSPARENT; };
    new ImageSet([standW]) { format = "../../graphics/zob/frames/stand_SE.bmp";        offset = "-13 -42"; flags = $TRANSPARENT; };
    new ImageSet([standN]) { format = "../../graphics/zob/frames/stand_N.bmp";         offset = "-13 -42"; flags = $TRANSPARENT; };
    new ImageSet([standS]) { format = "../../graphics/zob/frames/stand_S.bmp";         offset = "-13 -42"; flags = $TRANSPARENT; };

    new ImageSet([talkingE]) { format = "../../graphics/zob/frames/talk_SE_??.bmp";  offset = "-13 -42"; flags = $TRANSPARENT; };
    new ImageSet([talkingW]) { format = "../../graphics/zob/frames/talk_SE_??.bmp";  offset = "-13 -42"; flags = $TRANSPARENT; };
    new ImageSet([talkingN]) { format   = "../../graphics/zob/frames/talk_N_02.bmp";   offset = "-13 -42"; flags = $TRANSPARENT; };
    new ImageSet([talkingS]) { format = "../../graphics/zob/frames/talk_S_??.bmp";   offset = "-13 -42"; flags = $TRANSPARENT; };

    new ImageSet([raiseArmE])   { format = "../../graphics/zob/frames/raisearm_SE_??.bmp";     offset = "-13 -42"; flags = $TRANSPARENT; };
    new ImageSet([raiseArmN])   { format = "../../graphics/zob/frames/raisearm_N_??.bmp";     offset = "-13 -42"; flags = $TRANSPARENT; };
    new ImageSet([scanS])       { format = "../../graphics/zob/frames/scan_S_??.bmp";    offset = "-13 -42"; flags = $TRANSPARENT; };
    new ImageSet([aimE])        { format = "../../graphics/zob/frames/aim_E_??.bmp";           offset = "-13 -42"; flags = $TRANSPARENT; };
    new ImageSet([fireUpE])     { format   = "../../graphics/zob/frames/fireup_E.bmp";           offset = "-13 -57"; flags = $TRANSPARENT; };
    new ImageSet([beam])        { format   = "../../graphics/zob/frames/beam_??.bmp";           offset = "-13 -57"; flags = $TRANSPARENT; };
    new ImageSet([die])         { format  = "../../graphics/zob/frames/die_SE_??.bmp";     offset = "-13 -42"; flags = $TRANSPARENT; };
    new ImageSet([beamFx])      { format = "../../graphics/zob/frames/beam_??.bmp";            offset = "-13 -48"; flags = $TRANSPARENT; };
    
};

// ---------- Costume ----------
new Costume(ZobCostume)
{
    palette = zobPalette;
    flags   = FLIP;             // costume-wide
    limbs   = body, talk_body;  // two limbs
    baseTalkPos = 0, -48;

    // --- Anim: walk ---
    new CostumeAnim([walk])
    {
        E[body] = zob_ImageSets->walkingE.pick(1,4);
        W[body] = zob_ImageSets->walkingW.pick(1,4);
        S[body] = zob_ImageSets->walkingS.pick(1,4);
        N[body] = zob_ImageSets->walkingN.pick(1,4);
        E[talk_body] = $HIDE;
        W[talk_body] = $HIDE;
        S[talk_body] = $HIDE;
        N[talk_body] = $HIDE;
        // loops by default
    };

    // --- Anim: stand ---
    new CostumeAnim([stand])
    {
        E[body] = zob_ImageSets->standE.pick(0);
        W[body] = zob_ImageSets->standW.pick(0);
        N[body] = zob_ImageSets->standN.pick(0);
        S[body] = zob_ImageSets->standS.pick(0);
        E[talk_body] = $FLAG_SHOW;
        W[talk_body] = $FLAG_SHOW;
        S[talk_body] = $FLAG_SHOW;
        N[talk_body] = $FLAG_SHOW;
    };

    // --- Anim: init (same as stand) ---
    new CostumeAnim([init])
    {
        E[body] = zob_ImageSets->standE.pick(0);
        W[body] = zob_ImageSets->standW.pick(0);
        N[body] = zob_ImageSets->standN.pick(0);
        S[body] = zob_ImageSets->standS.pick(0);
    };

    // --- Anim: talkStart ---
    new CostumeAnim([talkStart])
    {
        E[talk_body] = zob_ImageSets->talkingE.pick(1,3);   // 0000..0002
        W[talk_body] = zob_ImageSets->talkingW.pick(1,3);
        S[talk_body] = zob_ImageSets->talkingS.pick(1,3);
        N[talk_body] = zob_ImageSets->standN.pick(0), zob_ImageSets->talkingN.pick(2), zob_ImageSets->standN.pick(0);
    };

    // --- Anim: talkStop ---
    new CostumeAnim([talkStop])
    {
        E[talk_body] = $HIDE;
        W[talk_body] = $HIDE;
        S[talk_body] = $HIDE;
        N[talk_body] = $HIDE;
    };

    // --- Anim: raiseArm  (@24, one-shot) ---
    new CostumeAnim([raiseArm])
    {
        id = 24;
        E[body] = zob_ImageSets->raiseArmE.pick(1,2);
        W[body] = zob_ImageSets->raiseArmE.pick(1,2);
        N[body] = zob_ImageSets->raiseArmE.pick(1,2);
        N[body] = zob_ImageSets->standN.pick(0), zob_ImageSets->talkingN.pick(2), zob_ImageSets->standN.pick(0);
        flags   = $NO_LOOP;
    };

    // --- Anim: lowerArm (@28, one-shot) ---
    new CostumeAnim([lowerArm])
    {
        id = 28;
        E[body] = zob_ImageSets->raiseArmE.pick(0), zob_ImageSets->standE.pick(0);
        W[body] = zob_ImageSets->raiseArmE.pick(0), zob_ImageSets->standW.pick(0);
        N[body] = zob_ImageSets->raiseArmE.pick(0), zob_ImageSets->standN.pick(0);
        N[body] = zob_ImageSets->standN.pick(0), zob_ImageSets->talkingN.pick(2), zob_ImageSets->standN.pick(0);
        flags   = $NO_LOOP;
    };

    // --- Anim: scan (@32) ---
    new CostumeAnim([scan])
    {
        id = 32;
        E[body] = zob_ImageSets->scanS.pick(1,2), zob_ImageSets->scanS.pick(1), $NO_LOOP;
        W[body] = zob_ImageSets->scanS.pick(1,2), zob_ImageSets->scanS.pick(1), $NO_LOOP;
        N[body] = zob_ImageSets->scanS.pick(1,2), zob_ImageSets->scanS.pick(1), $NO_LOOP;
        S[body] = zob_ImageSets->scanS.pick(1,2), zob_ImageSets->scanS.pick(1), $NO_LOOP;
    };

    // --- Anim: fireup (@40, one-shot) ---
    new CostumeAnim([fireup])
    {
        // id = 40;
        E[body] =  zob_ImageSets->aimE.pick(1,2), zob_ImageSets->fireUpE.pick(1);
        W[body] =  zob_ImageSets->aimE.pick(1,2), zob_ImageSets->fireUpE.pick(1);
        S[body] =  zob_ImageSets->aimE.pick(1,2), zob_ImageSets->fireUpE.pick(1);
        N[body] =  zob_ImageSets->aimE.pick(1,2), zob_ImageSets->fireUpE.pick(1);
        flags   = $NO_LOOP;
    };

    // --- Anim: beam (@44, one-shot) ---
    new CostumeAnim([beam])
    {
        id = 44;
        E[body] = zob_ImageSets->beamFx.pick(1,9), zob_ImageSets->standE.pick(0);
        W[body] = zob_ImageSets->beamFx.pick(1,9), zob_ImageSets->standW.pick(0);
        S[body] = zob_ImageSets->beamFx.pick(1,9), zob_ImageSets->standS.pick(0);
        N[body] = zob_ImageSets->beamFx.pick(1,9), zob_ImageSets->standN.pick(0);
        flags   = $NO_LOOP;
    };

    // --- Anim: firestraight (@52, one-shot) ---
    new CostumeAnim([die])
    {
        id = 52;
        E[body] =  zob_ImageSets->die.pick(1,8), $NO_LOOP;
        W[body] =  zob_ImageSets->die.pick(1,8), $NO_LOOP;
        S[body] =  zob_ImageSets->die.pick(1,8), $NO_LOOP;
        N[body] =  zob_ImageSets->die.pick(1,8), $NO_LOOP;
        flags   = $NO_LOOP;
    };
};

ZobCostume.compileCostume();