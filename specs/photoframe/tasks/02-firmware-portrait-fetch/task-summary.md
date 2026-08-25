# 02 — Firmware Portrait + Photo Fetch (summary)

The panel now fetches the real 480×800 PNG over HTTP, draws it upright and unscaled,
reports liveness to Home Assistant, and skips the redraw when the server says 304. All
three device-only measurements are recorded, and the bezel and palette questions the
server side was blocked on are answered.

## Actions

1. **Measured the panel palette** off the task 01 test pattern before overwriting it —
   see `panel-palette.md`. Six primaries, 8.3:1 contrast.
2. **Measured the bezel and settled the rotation** with a concentric-ring probe — see
   `bezel-and-rotation.md`. Inset 0–1 px, `rotation: 90°`, cable on the right.
3. **Built the fetch path**: `http_request`, `online_image` at RGB565 with no `resize:`,
   the `!cached` redraw guard, a `last_fetch_ok` timestamp, SNTP, and the hourly
   placeholder interval. Deleted the test pattern and the now-unused font.
4. **Moved `photo_url` into `secrets.yaml`** — this repo is public and the URL carries an
   internal hostname.
5. **Measured fetch+decode, peak RAM and real-photo refresh time** with temporary
   instrumentation, then removed it.

## Results

| | |
|---|---|
| Fetch + decode | 2672–2701 ms across four boots |
| PSRAM peak | ~795 KiB of 8 MiB — **~7 MiB spare** |
| Full refresh, real photo | 29 818 / 29 821 / 29 830 ms |
| Bezel inset | 0–1 px; no inset needed |
| Contrast | 8.3:1 |

**The escalation question is closed: RAM is a non-issue, so the server's hand-built
indexed PNG is not needed.** It existed only as a fallback in case RGB565 would not fit.

## Deviations from the plan

- **Went straight from the bezel probe to the fetch path** instead of reverting the probe
  first. Step 2 replaces the lambda either way, so reverting would have been churn plus a
  pointless 30 s refresh.
- **Used SNTP, not the `homeassistant` time platform** the plan specified. With the HA
  platform the device sat at `1970-01-01` forever, so `utcnow()` was invalid and the
  timestamp silently never published. More fundamentally: HA being unreachable is one of
  the failure modes this timestamp exists to reveal, so the clock must not depend on it.
- **Kept a one-line `stamp` log** rather than removing all instrumentation.
  `text_sensor::publish_state` logs only at VERBOSE, so without it there is no way to
  tell a publish from a silent skip — exactly the ambiguity that cost time here.
- **Folded step 3's instrumentation into the step 2 flash** to save a cycle, and used
  `heap_caps_get_minimum_free_size()` — the since-boot low-water mark — instead of
  sampling during the decode and hoping to catch the peak.

## Lessons

- **Count a landmark; don't measure a coordinate.** Every attempt to pin down absolute
  device→photo geometry in the bezel photo drifted 5–11 device px — larger than the
  quantity being measured. The ring probe worked because the answer was a *count*: three
  red rings or not three. That is a topological fact, immune to calibration error, and it
  matched the naked-eye count exactly.
- **Verify the instrument before trusting the reading.** The first palette attempt
  produced a physically impossible result — yellow reflecting more red than white — which
  is what exposed a specular sheen and 55% illumination gradient. Building in a check with
  a known answer is what turned a confident wrong number into a right one.
- **A missing log line is not evidence.** I inferred `last_fetch_ok` was never publishing
  because no state line appeared. `publish_state` logs at VERBOSE; the publish was fine.
  Check the log level before concluding from silence.
- **Shoot RAW; the phone's JPEG is unusable for measurement.** HDR+ tone-maps locally, so
  each patch's value depends on its surroundings and the readings stop being comparable.
- **Read the component source, not the docs** — held again. `cached` as the trigger
  variable, `format` required but `resize` optional, `(0x50, 0x3F)` as the fixed border
  byte, and `get_width()` swapping under `SWAP_XY` all came from source, not docs.
- **A killed `make run` leaves its container holding the serial port.** Every
  "device disconnected or multiple access on port?" upload failure traced to a leftover
  `esphome-usb` container from a timeout-killed run — not the flaky cable I first
  suspected. `docker rm -f` on stale containers fixes it instantly. Clean up before
  flashing.
- **ESPHome logs a healthy 304 as an error** (`[E] HTTP Request failed … Code: 304`, plus
  a briefly-set error flag). Cosmetic, but it is the steady state once the photo stops
  changing, so it will be misread as a fault by anyone reading these logs later.

## Not done

- **Failure test not run.** Needs the server container on the Pi stopped, which I cannot
  do from here. The code path is reachable only from `on_download_finished`, so the panel
  physically cannot be cleared by a failed fetch — but that is reasoning, not a
  measurement, and task 01's lesson was that silent failures do not announce themselves.
- **HA reporting unverified end to end.** The device publishes `last_fetch_ok` but has
  never been adopted in Home Assistant, so nothing is receiving it. HA itself is up.
- **Scheduled redraw unverified.** With a fixed `current.png`, the only path that runs
  after boot is the 304 branch. Confirming a *scheduled* redraw needs the server's daily
  rotation.

## Architecture doc

Updated: rotation and the cable-on-the-right constraint, measured RAM headroom and refresh
time, the corrected bezel finding, the 304 guard, why liveness is a timestamp, and why the
clock is SNTP. Three new open items: HA adoption, the SNTP-versus-boot-fetch race that
would break stamping under deep sleep, and `online_image`'s main-loop blocking.
