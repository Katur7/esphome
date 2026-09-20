# Handover → photoframe server repo: source-line crosstalk from the task-05 render

Device-side finding, 2026-08-30. The tuned (`05-the-look`) render produces visible streaks
on the panel. **Firmware is unchanged and not the cause.** The fix belongs in the dither.

## Symptom

After a full refresh the panel shows faint 1-px lines of off colour. Portrait-hung they run
horizontal; in `crosstalk-panel.jpg` (shot landscape, FPC at top) they run vertical. Dark
regions come out dark brown-red instead of black, and the streaks continue out of the dark
region into adjacent light content (e.g. down from the hair into the sleeve).

Observed on two consecutive tuned renders (ETags `W/"165aa-…"`, `W/"189b4-…"`). Not seen
with the pre-task-05 render (ideal inks, sRGB, no tone compression).

## What was ruled out on the device

- Same binary as the last good image; config clean in git.
- Served PNG verified: 480×800, RGB8, non-interlaced, exactly the six pure corners. The
  `colorconv` classifier passes them through 1:1.
- `epaper_spi` waits on BUSY with no timeout — a refresh cannot be cut short. ~30 s, normal
  E6 blink sequence.
- Init sequence is Waveshare's stock 7.3" E6 table. Nothing in it is tunable for this.
- No even/odd-row or column bias in the PNG (per-colour counts per row parity within <1%),
  so serpentine is not producing banding.

## Evidence it is the panel reacting to the render

`crosstalk-side-by-side.jpg`: left = photographed panel (DNG developed, registered to the
active area), right = same region of the served PNG, rotated to match.

- PNG: a **solid pure-black** area with a sparse sprinkle of isolated red and green pixels
  — how the tuned pipeline renders near-black under black-subtracted measured inks.
- Panel: the same area averages RGB(70,59,59) and is covered in continuous 1-px streaks,
  **including across the patch where the PNG has no coloured pixels at all**.
- Streak axis = panel source lines (the 480 axis). An isolated red/green pixel in a black
  column leaks its waveform into every black pixel sharing that source line.

Classic Spectra 6 source-line crosstalk. Content-dependent: it needs isolated saturated
pixels inside large black fields, which the old render never produced.

## Probe

`crosstalk-probe.png` (480×800):
- rows 0–259: solid black — control, should stay clean
- rows 280–539: black with one red pixel every 8th row at x=240 — expect full-width red
  streaks along those rows
- rows 560–799: black with a 2 % red/green sprinkle like the tuned render — expect the
  brown-striped look

Serve it as `current.png` for one refresh to confirm before changing the pipeline.

## Suggested fixes, in order

1. **Black floor in the diffuser.** Anything within a small ΔE of black snaps to black with
   no error carried. Kills the sprinkle; cheapest and probably most of the win.
2. **Revisit black-subtracted inks.** Telling the diffuser black is `#000` is what makes it
   reach for red/green dots to hit the panel's real Y 0.121. Raw measured black (`#666064`)
   or a partial subtraction avoids the fiction without the sprinkle.
3. **Generalise:** the same leak applies to any isolated ink in a field of a different ink.
   Prefer clustered/coarser structure in flat fields over single-pixel dots — this is a
   panel constraint the screen's noise metric does not see.
4. **Model it in `simulatePanel`**: smear a fraction of each pixel's colour along the source
   axis (portrait rows). The current sim showed none of this and looked fine.

## Also

The whole image reads strongly green and pale on the panel compared to the sim. Separate
issue; judge it after the streaks are gone.

Files alongside this doc: `crosstalk-panel.jpg`, `crosstalk-side-by-side.jpg`,
`crosstalk-probe.png`. The DNG (`specs/photoframe/PXL_20260830_134947471.RAW-02.ORIGINAL.dng`,
22 MB) is not to be committed.
