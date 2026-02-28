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
new Palette(carolPalette)
{
    source = "../../graphics/rooms/back01_merged.bmp";
};

// ---------- ImageSets (pictures) ----------

new SimSet(carol_ImageSets)
{
    new ImageSet([walk])
    {
        format = "../../graphics/carol/walkfrombehindwall_??.bmp";
        offset = "-40 -56";
    };
    new ImageSet([clean])
    {
        format = "../../graphics/carol/clean_??.bmp";
        offset = "-40 -56";
    };
};

// ---------- Costume ----------
new Costume(CarolCostume)
{
    palette = carolPalette;
    //flags   = FLIP;          // costume-wide
    limbs   = body;          // single limb
    baseTalkPos = 0, -56;

    // --- Anim: stand ---
    new CostumeAnim([stand])
    {
        S[body] = carol_ImageSets->clean.pick(1, 3), carol_ImageSets->clean.pick(1), $LOOP;
    };

    // --- Anim: init ---
    new CostumeAnim([init])
    {
        S[body] = carol_ImageSets->walk.pick(1, 5), carol_ImageSets->clean.pick(1), $NO_LOOP;
    };

    // --- Anim: talkStart ---
    new CostumeAnim([talkStart])
    {
        S[body] = carol_ImageSets->clean.pick(1);
    };

    // --- Anim: talkStop ---
    new CostumeAnim([talkStop])
    {
        S[body] = carol_ImageSets->clean.pick(1, 3), carol_ImageSets->clean.pick(1), $LOOP;
    };

    // --- Anim: walkIntoRoom ---
    new CostumeAnim([walkIntoRoom])
    {
        S[body] = carol_ImageSets->walk.pick(1, 6);
    };

    // --- Anim: clean ---
    new CostumeAnim([clean])
    {
        S[body] = carol_ImageSets->clean.pick(1, 3), $LOOP;
    };

};

CarolCostume.compileCostume();
