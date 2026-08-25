# Measured bezel inset and rotation — 7.3" Spectra 6

Step 1 of task 02. Answers two questions the config could not: how much of the drawing
surface disappears under the bezel, and which way round `rotation: 90°` actually lands.

Measured 2026-08-25 from `PXL_20260825_070426623.RAW-02.ORIGINAL.dng` (kept out of git;
`bezel-probe.jpg` is the preview).

## Findings

**Bezel inset is 0–1 px. Nothing is clipped.** Content can run to the very edge of the
480×800 surface with no inset. This corrects the ARCHITECTURE's earlier "~1–2 px at the
extreme edge sits under the bezel", which was an eyeball estimate from the task 01 photo.

**The pale margin around the image is not lost pixels.** It is the panel's
non-addressable border region, outside the 800×480 matrix, driven by the controller's
border setting rather than written from the framebuffer — which is why it visibly
changes colour during a refresh. It reads light and works as an accidental mat around
the photo.

It is **not configurable from YAML**. ESPHome's `spectra_e6` init sequence hardcodes one
byte, `(0x50, 0x3F)` — the VCOM / data-interval command, where border behaviour lives on
this controller family. The models that do expose a border waveform option (`ssd1677`,
`waveshare_b`, `weact_bwr`) are not this panel. Deliberately colouring the border would
mean patching the component.

**`rotation: 90°` is correct, and the frame must be built with the cable on the right.**
The device reports `drawing surface is 480x800` at boot, and the probe's corner blocks
put device origin at the panel's physical **top-right** in landscape. So the image is
upright when the panel is turned 90° counter-clockwise from landscape, which puts the
FPC ribbon and driver board on the **right-hand** side of the portrait frame.
`rotation: 270°` is the mirror of that, with the cable on the left.

## Method, and the two things that went wrong

The probe drew 17 concentric 2 px rings from the outermost pixel inward, cycling
red/green/blue/yellow/black with a white gap every sixth slot, plus four
differently-coloured 40×40 corner blocks.

**Locating the surface by corner blocks failed.** Colour-blob centroids put the blocks
badly out — measured block sizes came out 0 to 258 photo px where 144 was expected — so
any homography built on them was unusable. Four correspondences also determine a
homography exactly, so its zero reprojection error is not evidence of anything. The
block-size check is what caught it.

**Per-pixel palette classification also failed.** At 3.7 photo px per device px, lens
blur mixes adjacent 2 px rings, so the narrow black ring reads as green and the sequence
comes out scrambled.

What worked was counting a landmark that does not depend on either. The pattern contains
**exactly three red rings** (slots 0, 6, 12) and three yellow (slots 3, 9, 15). Red has
by far the highest R/(R+G+B) chromaticity of the six primaries — 0.63 against 0.47 for
the next nearest — and chromaticity is immune to the illumination gradient. Peaks along
each edge alternate red/yellow every ~6 device px, and separating them by chroma gives
**three red and three yellow on all four edges**: the complete pattern, so the outermost
2 px ring is present and nothing is hidden. Confirmed independently by eye.

## Lesson

**Count a landmark, don't measure a coordinate.** Every attempt to establish absolute
device→photo geometry drifted by 5–11 device px, which is larger than the quantity being
measured. The ring count is a topological fact — three reds or not three reds — and it
survived calibration error entirely. Design the diagnostic so the answer is a count.
