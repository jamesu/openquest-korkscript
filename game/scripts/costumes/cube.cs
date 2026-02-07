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
new Palette(cubePalette)
{
    source = "../graphics/rooms/back02_merged.bmp";
};

// ---------- ImageSets (pictures) ----------

new SimSet(cube_ImageSets)
{
    new ImageSet([standS])
    {
        format = "../graphics/cube/cube_??.bmp";
        offset = "-17 -17";
    };
};

// ---------- Costume ----------
new Costume(CubeCostume)
{
    palette = cubePalette;
    //flags   = FLIP;          // costume-wide
    limbs   = body;          // single limb

    // --- Anim: stand ---
    new CostumeAnim(stand)
    {
        S[body] = cube_ImageSets->standS.pick(0);
    };

    // --- Anim: init (same as stand) ---
    new CostumeAnim(init)
    {
        S[body] = cube_ImageSets->standS.pick(0);
    };

    // --- Anim: dissolve ---
    new CostumeAnim(dissolve)
    {
        S[body] = cube_ImageSets->standS.pick(0, 7), NO_LOOP;
    };

};
