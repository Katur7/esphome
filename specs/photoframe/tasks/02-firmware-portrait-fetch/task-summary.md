# 02 — Firmware Portrait + Photo Fetch (summary)

Panel fetches the real 480×800 PNG, draws it upright and unscaled, stamps liveness for HA,
and skips the redraw on a 304. All three device-only measurements recorded; the bezel and
palette questions the server side was blocked on are answered.

## Actions

1. Measured the panel palette off the task 01 test pattern before overwriting it —
   `panel-palette.md`. Six primaries, 8.3:1 contrast.
2. Measured bezel and settled rotation with a concentric-ring probe —
   `bezel-and-rotation.md`. Inset 0–1 px, `rotation: 90°`, cable on the right.
3. Built the fetch path: `http_request`, `online_image` at RGB565 with no `resize:`, the
   `!cached` redraw guard, `last_fetch_ok`, SNTP. Deleted test pattern and font.
4. Moved `photo_url` into `secrets.yaml` — public repo, internal hostname.
5. Measured fetch+decode, peak RAM and real-photo refresh with temporary instrumentation,
   then removed it.

## Results

| | |
|---|---|
| Fetch + decode | 2672–2701 ms across four boots |
| PSRAM peak | ~795 KiB of 8 MiB — **~7 MiB spare** |
| Full refresh, real photo | 29 818 / 29 821 / 29 830 ms |
| Bezel inset | 0–1 px; no inset needed |
| Contrast | 8.3:1 |

**Escalation question closed: RAM is a non-issue, so the server's hand-built indexed PNG is
not needed.** It existed only as a fallback in case RGB565 would not fit.

## Deviations from the plan

- **Probe not reverted before step 2** — step 2 replaces the lambda either way, so
  reverting would have been churn plus a pointless 30 s refresh.
- **Added a `time:` block the plan did not specify.** The plan wanted an ISO8601 stamp
  without naming a clock. The `homeassistant` platform left the device at `1970-01-01`
  forever, so `utcnow()` was invalid and the stamp silently never published. SNTP instead —
  and HA being unreachable is one of the failure modes this stamp exists to reveal, so the
  clock must not depend on it.
- **Step 3 instrumentation folded into the step 2 flash** to save a cycle, using
  `heap_caps_get_minimum_free_size()` (since-boot low-water mark) rather than sampling
  mid-decode and hoping to catch the peak.

## The failure test found a real bug

An unreachable server **reboot-looped the device**: 6 fetch attempts, 6 task-watchdog
panics, 6 reboots, safe mode counting down, `on_error` never reached. IDF `http_request`
blocks the main loop during connect, and the watchdog fires at 5 s — long before a 30 s
HTTP timeout could fail gracefully. It does wrap the request in a `WatchdogManager`, but
`watchdog_timeout_` defaults to `0` and `WatchdogManager` returns early on zero, so the
protection is inert unless configured.

Fixed with `watchdog_timeout: 20s` + `timeout: 10s` — the ceiling must sit above the
request timeout. Verified: 3 failures at exactly 10.03 s, `on_error` firing,
`last_fetch_ok` frozen, **0 reboots, 0 redraws**.

My pre-test reasoning — the panel is safe because `component.update: epaper` is only
reachable from `on_download_finished` — was right about the panel and blind to the crash
loop. Measure failure paths; don't reason about them.

## Lessons

- **Count a landmark; don't measure a coordinate.** Absolute device→photo geometry drifted
  5–11 device px, larger than the quantity being measured. The ring probe worked because
  the answer was a *count* — three red rings or not. Topological, immune to calibration
  error, and it matched the naked eye.
- **Verify the instrument first.** The first palette attempt gave a physically impossible
  result — yellow reflecting more red than white — which is what exposed the specular sheen
  and 55% illumination gradient. A check with a known answer is what caught it.
- **A missing log line is not evidence.** `publish_state` logs at VERBOSE; the publish was
  fine all along.
- **Shoot RAW.** HDR+ tone-maps locally, so patch values depend on their surroundings and
  stop being comparable.
- **Read the component source, not the docs** — held again. `cached`, `resize` being
  optional, `(0x50, 0x3F)`, and `get_width()` swapping under `SWAP_XY` all came from source.
- **A killed `make run` leaves its container holding the serial port.** Every "multiple
  access on port?" failure traced to a leftover `esphome-usb` container, not the flaky
  cable I first suspected. `docker rm -f` fixes it instantly.
- **ESPHome logs a healthy 304 as an error.** Cosmetic, but it is the steady state once the
  photo stops changing, so it will be misread as a fault later.

## Post-review changes

A `/deep-review` pass after the work landed produced these:

- Split `http_request` / `time` / `image` / `text_sensor` out into `photo.package.yaml`;
  `display.package.yaml` is back to SPI + display.
- Replaced `script:` + `interval:` with `update_interval: 1h` on `online_image`. Its first
  tick is at +1 h, not boot, so `on_boot` owns the initial fetch.
- **`on_boot` now waits for a valid clock as well as wifi.** SNTP lands 0–30 s after boot
  and raced the first fetch — observed stamping on the first fetch on two boots and
  skipping it on another. Collapsed the stamp to one declarative
  `text_sensor.template.publish`; the `is_valid()` guard and the `stamp` log mirror are
  gone with it.
- Deleted the camera RAWs and their `.gitignore` lines; the derived numbers in
  `panel-palette.md` are the deliverable.
- Two documented facts were wrong and are corrected: the SNTP deviation above (the plan
  named no clock source, so this was an addition, not a substitution) and the rotation
  evidence in `bezel-and-rotation.md` (corner *colour* identity, not the block geometry
  that same document shows is unusable).

Two findings were left alone deliberately, both recorded in ARCHITECTURE: the upstream
`online_image` `len == 0` wedge, and the 304-versus-busy-display race that the current
single serialized trigger path makes unreachable.

## Not done

- **HA adoption deferred by decision, not overlooked.** Adopting now would register an
  always-on device; once it sleeps through most of the day HA would read that as
  permanently unavailable. Adopt after the wake schedule exists. So the reporting path is
  unverified end to end — HA itself is up.
- **Scheduled redraw unverified.** With a fixed `current.png` the only post-boot path is
  the 304 branch. Confirming a *scheduled* redraw needs the server's daily rotation.
- **Post-review config unflashed.** The board was disconnected when these changes were
  made; the clock-wait and the new boot path are compile-verified only.

## Architecture doc

Updated: rotation and cable-on-the-right, measured RAM headroom and refresh time, the
corrected bezel finding, the 304 guard, liveness-as-timestamp, and why the clock is SNTP.
Open items: HA adoption, the SNTP/boot-fetch race, `online_image` main-loop blocking, the
`len == 0` wedge, and the single-trigger-path invariant.
