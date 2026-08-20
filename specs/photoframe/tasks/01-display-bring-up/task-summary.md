# 01 — Display Bring-Up (summary)

Panel works. All six colour bands correct and in order, four corner markers present, border complete, text legible.

## Actions

1. Stripped the config to the minimum that can draw — SPI, one font, display, test-pattern lambda. Image pipeline commented out in place, not deleted
2. Added an `esphome-usb` service to `compose.yaml` and a `USB=1` switch to the Makefile — the default service cannot see `/dev`, so USB flashing was impossible
3. Flashed over USB, captured boot logs, fixed the display config, reflashed

## What was wrong

Two bugs, both in the display block, both found by reading the installed component source rather than the docs:

- **Model string.** Used the generic `Spectra-E6` controller model. The panel-specific `7.3in-Spectra-E6` exists and carries `data_rate: 20MHz`; the generic one left SPI at the 10MHz default.
- **Busy polarity.** Had `inverted: false`. E6 busy is active-low — every Spectra-E6 variant in `esphome/components/epaper_spi/models/spectra_e6.py` sets `inverted: true` with a pullup, and Seeed's own EE04 config agrees.

## Lessons

- **A wrong busy polarity does not announce itself.** The logs were clean, the component reported `Display update took 981 ms`, and the panel stayed blank. Nothing errored. The tell was the timing: 981 ms is impossibly fast for an E6, and after the fix the same refresh took **31207 ms**. Treat a suspiciously fast e-paper refresh as a failure, not a success.
- **Read the component source, not the docs.** `esphome/components/epaper_spi/models/spectra_e6.py` gave the exact model names, default data rates, and the busy-pin convention. The ESPHome docs listed only the generic controller model, and the docs' example used `inverted: false` — which is what led me wrong in the first place.
- **Issue #12322 was a red herring.** Closed stale, no fix, and did not reproduce here — zero reboots.
- **Docker had no device access.** einkframe has always used OTA, so this never came up. `docker compose run` has no `--device` flag; it has to be a compose service. Kept as a *separate* privileged service so the default path stays unprivileged.
- **The XIAO needs its U.FL antenna physically attached.** Without it the scan returns zero networks — not "SSID missing", literally no APs. Easy to misread as a credentials problem.
- **Test patterns should be diagnostic.** Each element mapped to one question (corners → geometry, bands → palette, border → clipping, text → fonts), so the single visual check answered all four at once.

## Hardware notes

- PSRAM confirmed present at boot: `Available: YES, Size: 8192 KB`
- Full-frame colour refresh: **~31 s**. Drives everything about refresh cadence and battery life
- ~1–2 px at the extreme edge sits under the panel bezel. Keep content inset
- Charge LED blinking rapidly with no battery attached is the ETA6003's no-battery detection, not a fault

## Architecture doc

Updated: busy polarity, model string, data rate, refresh duration, and PSRAM all moved from open questions to confirmed facts.
