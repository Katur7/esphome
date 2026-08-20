# 01 — Display Bring-Up

## Goal

Get pixels on the panel. Prove the hardware and the `epaper_spi` config before any image pipeline work.

## What this test answers

- Does a refresh complete at all, or does it hang on the busy pin? (polarity is unverified)
- Does the 7.3" E6 bootloop, per ESPHome issue [#12322](https://github.com/esphome/esphome/issues/12322)?
- Do all six palette colors come out right, and in the order we expect?
- Is the full 800×480 addressable, with origin at top-left?
- Does text render?

## Approach

Strip the config to the minimum that can draw: SPI, one font, the display, a test-pattern lambda. The `http_request` / `online_image` / refresh-script block is commented out in place, not deleted — re-enabling it later is uncommenting, and it keeps the HTTP stack, PNG decoder, and an unreachable-URL warning out of a first-light test.

Test pattern: six labelled colour bands, a corner marker in each of the four corners, a full-frame border, and a title line. Each element maps to one question above — a missing corner means the geometry is wrong, a wrong band means the palette mapping is.

Drawn once per boot (`update_interval: never` + `component.update` in `on_boot`). Reset button redraws. Deliberately not on a timer — a colour refresh takes tens of seconds.

## Success

Panel shows the pattern, all six bands correct, all four corners present, no bootloop.

## Follow-ups (not this task)

- Restore the image pipeline, then build whatever serves the PNG
- Deep sleep + battery (renumbered to task 02)
