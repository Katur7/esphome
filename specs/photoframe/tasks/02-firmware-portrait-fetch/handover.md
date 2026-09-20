# Handover: check firmware/driver levers for Spectra-6 line crosstalk

For the agent working in the esphome repo (`specs/photoframe/`). Read-only investigation —
report what exists, change nothing yet.

## What the panel does (measured, server-side probes)

On the 480×800 Spectra 6 panel, any source line (portrait render row) whose content is
**dithered blue+yellow** gets a red-brown wash over its own black pixels — the whole line,
gated line-exactly by the line's own content. Solid blue ≈ no effect, solid yellow small,
scattered blue+yellow strong. There is also a slow build-up over ~400 consecutive such
lines, and the exact wash amplitude depends on the exact pixel arrangement, not just
counts. Everything is fully deterministic: same PNG → same artifact, every refresh.

Working theory: dithered b+y forces the source driver to swing between the two most
extreme drive voltages on nearly every gate step; the dirtied waveform corrupts the black
drive on the same line. Content-side mitigation is underway server-side. The firmware
question is whether the panel is being driven with the right waveform at all.

## What to check

1. **Which display component/driver** the photoframe config uses, and which controller it
   assumes. Is it a generic Spectra-6/AC057-style driver or one written for this exact
   panel model?
2. **Waveform/LUT source**: does the driver load the vendor waveform from the panel's own
   OTP/flash, or does it hardcode an init sequence + LUT in the component? If hardcoded,
   where did it come from, and is there a newer vendor reference?
3. **Temperature**: does the driver read the panel's temperature sensor and set the
   temperature register before refresh? A wrong temp band selects wrong waveform timing —
   worth knowing what it does at, say, 18–25 °C indoor.
4. **Booster/VCOM/init registers**: do the power settings match the vendor sample code for
   this panel, or a sibling panel's?
5. **Refresh/update modes**: is more than one full-refresh mode available (slower/cleaner
   vs faster)? Any per-refresh options we're not using?
6. **Data clocking**: SPI speed vs vendor reference — long shot, listed for completeness.

## Report back

Per lever: exists / configurable / already-correct / unknown. No changes — the server side
needs to know which knobs are real before deciding whether a firmware experiment is worth
a panel session.
