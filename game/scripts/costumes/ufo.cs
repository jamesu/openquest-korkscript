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
new Palette(ufoPalette)
{
    source = "../graphics/rooms/skyline.bmp";
};

// ---------- ImageSets (pictures) ----------

new SimSet(ufo_ImageSets)
{
    new ImageSet([ufo])
    {
        format = "../graphics/ufo/ufo_??.bmp";
        offset = "-40 -56";
    };
};

// ---------- Costume ----------
new Costume(UfoCostume)
{
    palette = ufoPalette;
    //flags   = FLIP;          // costume-wide
    limbs   = body;          // single limb

    // --- Anim: walk ---
    new CostumeAnim(walk)
    {
        E[body] = ufo_ImageSets->ufo.pick(0,2), LOOP;
        W[body] = ufo_ImageSets->ufo.pick(0,2), LOOP;
        S[body] = ufo_ImageSets->ufo.pick(0,2), LOOP;
        N[body] = ufo_ImageSets->ufo.pick(0,2), LOOP;
    };

    // --- Anim: stand ---
    new CostumeAnim(stand)
    {
        E[body] = ufo_ImageSets->ufo.pick(0,2), LOOP;
        W[body] = ufo_ImageSets->ufo.pick(0,2), LOOP;
        N[body] = ufo_ImageSets->ufo.pick(0,2), LOOP;
        S[body] = ufo_ImageSets->ufo.pick(0,2), LOOP;
    };

    // --- Anim: init (same as stand) ---
    new CostumeAnim(init)
    {
        E[body] = ufo_ImageSets->ufo.pick(0,2), LOOP;
        W[body] = ufo_ImageSets->ufo.pick(0,2), LOOP;
        N[body] = ufo_ImageSets->ufo.pick(0,2), LOOP;
        S[body] = ufo_ImageSets->ufo.pick(0,2), LOOP;
    };
};
