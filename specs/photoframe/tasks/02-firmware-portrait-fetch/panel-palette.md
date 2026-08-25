# Measured panel palette — Waveshare 7.3" Spectra 6 (E6)

Step 0 of task 02. These are the six primaries this panel actually produces, measured
off the task 01 bring-up test pattern. They are the working palette the server's dither
needs; without them the dither is aiming at idealised colours the panel cannot make.

Measured 2026-08-25 from two camera RAWs — `PXL_20260825_064535775` (primary, tilted) and
`PXL_20260825_064457857` (cross-check, straight above). **The RAWs are not retained**;
they were ~20 MB each and the numbers below are the deliverable. Re-measuring means
re-shooting, so the method section is written to be repeatable.

## The palette

Linear values are reflectance ratios relative to the panel's **own white**, which is the
only meaningful reference — the panel cannot go brighter than it. `Y` is relative
luminance. `±` is the disagreement between the two independent frames, in sRGB units.

| band | linear R G B | Y | sRGB | hex | ± |
|---|---|---|---|---|---|
| black | 0.133 0.117 0.127 | 0.121 | (102, 96,100) | `#666064` | 11 |
| white | 1.000 1.000 1.000 | 1.000 | (255,255,255) | `#FFFFFF` | 0 |
| red | 0.379 0.127 0.096 | 0.178 | (166,100, 87) | `#A66457` | 2 |
| yellow | 1.010 0.827 0.317 | 0.829 | (255,235,153) | `#FFEB99` | 5 |
| blue | 0.271 0.340 0.562 | 0.341 | (142,158,198) | `#8E9EC6` | 7 |
| green | 0.219 0.286 0.227 | 0.267 | (129,146,131) | `#819283` | 3 |

**Contrast white:black = 8.3:1.** That is the entire dynamic range available.

Same values with black subtracted and white normalised per channel, if a full-range
working palette is wanted instead:

| band | hex | | band | hex |
|---|---|---|---|---|
| black | `#000000` | | yellow | `#FFE880` |
| white | `#FFFFFF` | | blue | `#6F8ABB` |
| red | `#911C00` | | green | `#59795F` |

## What this means for the dither

- **Luminance ladder is black 0.121 → red 0.178 → green 0.267 → blue 0.341 → yellow
  0.829 → white 1.000.** Note blue is *lighter* than green and red, which is the
  opposite of the usual assumption.
- **There is a hole between 0.341 and 0.829.** No primary lands in the upper midtones,
  so every value in that range must come from dithering blue/green against yellow/white.
  That region will be the noisiest part of any image and is where tuning effort pays off.
- **8.3:1 contrast means the source photo must be tone-compressed before dithering.**
  Feeding it full-range sRGB collapses everything below ~12% into the single black
  primary.
- The panel is far less saturated than the idealised Spectra 6 palettes published
  online (e.g. yellow as `#FFF338`). Those are vendor swatches, not measurements of
  this panel. Use the table above.

## Method, and how much to trust it

Developed from DNG with camera white balance and no auto-brighten, sampled in **linear**
light. The four 30×30 corner markers in the test pattern were located by dark-centroid
and used to fit a homography, so sampling happens in device coordinates and the tilt's
perspective is absorbed rather than approximated.

Illumination across the panel still varies ~58%, so it is modelled as a quadratic
surface fitted to 42 white-background patches (the pattern's background is the same
white pigment as the white band) and divided out. Band values are then anchored on the
white band itself, which is a direct measurement of white pigment at the band row.

Three independent checks that the result is sound:

- **Yellow's red channel measures 1.010 × white.** Theory says an ideal yellow reflects
  red essentially as well as a white pigment, so ≈1.00 is the expected answer and a good
  falsification test. It is the check that caught the bad first attempt.
- **White-band self-check is 1.6%** against the fitted illumination surface — an
  independent validation, since the surface is fitted only to background, not to the band.
- **Two frames shot minutes apart, at different angles, agree within 2–7 sRGB units** on
  everything except black (11).

Black is the weakest number. Any veiling glare off the semi-gloss surface adds light,
which lifts black and compresses contrast, so **8.3:1 is a lower bound** — the real
panel is at least this good, possibly slightly better.

## Lessons for the next time a panel needs measuring

- **Shoot RAW and ignore the phone's JPEG.** The first attempt's JPEG carried
  `HDR+ 1.0`, whose local tone mapping makes each band's value depend on its
  surroundings — the six readings stop being mutually comparable and no amount of
  post-hoc correction recovers them.
- **Tilt the panel ~10–15° off perpendicular.** The straight-above frame put a specular
  reflection on the active area and read black 24% too light (6.9:1 instead of 8.3:1).
  Tilting moves the reflection out of frame and costs nothing, because the corner
  markers let a homography undo the perspective exactly.
- **Indirect daylight was the fix.** The first attempt, under a directional indoor lamp,
  had a specular sheen over the middle bands and could not be rescued by modelling.
- **The test pattern's own white background is the flat-field reference.** No grey card
  or white paper needed, and paper would be worse — optical brighteners make it read
  brighter and bluer than e-paper white.
- **Don't sample near the corner markers.** Patches overlapping them silently poisoned
  the first illumination fit (15% RMS instead of ~2%) and produced physically impossible
  results before the cause was obvious.
