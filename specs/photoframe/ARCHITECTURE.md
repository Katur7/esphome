# Architecture

## Overview

ESPHome config for **photoframe** — a battery-powered 7.3" color e-paper photo frame, **hung portrait**. Fetches a pre-rendered 480×800 PNG over HTTP and draws it. The device does no photo selection and no image processing: whatever serves the URL owns picking, cropping, and palette-mapping.

The panel is natively 800×480 landscape; rotated 90° for portrait mounting the drawing surface is **480 wide × 800 tall**, which is what the served PNG must be.

Distinct from [einkframe](../einkframe/ARCHITECTURE.md), which is a mono *dashboard* frame. Same driver-board family, different panel and different job.

## File Structure

```
config/
  photoframe.yaml                     # Entry point: substitutions, PSRAM, flash,
                                      # on_boot (owns the initial fetch)
  packages/
    base.package.yaml                 # Shared: wifi, OTA, API, logging
    photoframe/
      display.package.yaml            # SPI + epaper_spi
      photo.package.yaml              # http_request, sntp, online_image,
                                      # last_fetch_ok sensor
```

No custom C++ yet — nothing has needed extracting.

## Hardware

- **Board**: Seeed XIAO ePaper Display Board **EE04** + XIAO ESP32-S3 Plus (16MB flash, PSRAM). ESP-IDF framework, `esp32-s3-devkitc-1` variant.
- **Panel**: Waveshare 7.3" Spectra 6 (E6), 800×480, 6-color. Uses the EE04's **50-pin** FPC connector — set the jumper accordingly. The 24-pin connector carries the 1.54"–7.5" panels instead. (The EE05 board is 24-pin only and cannot drive this panel.)
- **Battery**: 2000mAh Li-ion on the onboard JST 2.0 connector, with onboard charging IC and power switch. Same cell as einkframe.

### Pinout

Identical to the TRMNL board einkframe runs on — same Seeed driver-board family, so these are proven, not guessed.

| Function | Pin |
|---|---|
| SPI CLK | GPIO7 |
| SPI MOSI | GPIO9 |
| CS | GPIO44 |
| DC | GPIO10 |
| RST | GPIO38 |
| BUSY | GPIO4 |
| Battery ADC | GPIO1 (gated by GPIO6) |
| KEY0 / KEY1 / KEY2 | GPIO2 / GPIO3 / GPIO5, active low |

Battery ADC and the three buttons are wired but unused so far.

## How It Fits Together

- Display uses the mainline **`epaper_spi`** component, *not* `waveshare_epaper`. Model is **`7.3in-Spectra-E6`**, the panel-specific model — it supplies 800×480 and `data_rate: 20MHz`. The generic `Spectra-E6` controller model leaves SPI at the 10MHz default; don't use it here.
- **Busy pin is active-low**: `inverted: true` with a pullup. Verified on hardware. Getting this wrong fails silently — the driver never waits, logs a ~1s "successful" refresh, and leaves the panel blank. A real full refresh takes **~30 s**; anything much faster means it isn't actually driving the panel.
- **Requires ESPHome ≥ 2026.8.0** — `epaper_spi` is not in earlier releases. `compose.yaml` tracks `:latest`, so a stale local image will fail validation with confusing schema errors. `docker compose pull esphome` fixes it.
- **PSRAM is mandatory**, declared as `mode: octal, speed: 80MHz`. The 800×480 6-color frame buffer (~192KB at 4bpp) plus the decoded `online_image` buffer do not fit in internal RAM. Confirmed at boot: `Available: YES, Size: 8192 KB`.
- **Portrait via `rotation: 90°`** on the display block, so the drawing surface is 480 wide × 800 tall — confirmed on hardware, the device logs `480x800`. Device origin lands at the panel's physical top-right in landscape, which means **the frame must be built with the FPC ribbon on the right**. `rotation: 270°` is the mirror, cable on the left.
- **Measured headroom.** RGB565 480×800 costs exactly 768000 bytes of PSRAM; peak during fetch+decode is ~795KB, leaving ~7MB of the 8MB free. Fetch+decode is ~2.7s. Internal RAM peak is ~21KB above baseline.
- Refresh path: `on_boot` waits for wifi **and a valid clock** → `component.update: photo` downloads the PNG → `on_download_finished` stamps `last_fetch_ok` and, **only if `!cached`**, fires `component.update: epaper`. After boot, `online_image`'s own `update_interval: 1h` drives it. The display stays `update_interval: never` — it is only ever driven from `on_download_finished`.
- **`online_image`'s first poll is at +1h, not at boot.** `PollingComponent::start_poller()` registers a plain `set_interval`, which fires after one interval rather than immediately. That is why `on_boot` owns the initial fetch and the poller only covers the steady state — dropping `on_boot` would leave the panel blank for an hour.
- **Exactly one trigger path reaches the display, and the 304 guard depends on it.** `EPaperBase::update()` early-returns with an `ESP_LOGE` when the display is not `IDLE`, and `online_image` commits `etag_`/`last_modified_` *before* firing `on_download_finished`. So a redraw dropped for a busy display would be followed by 304s forever — permanently stale. Today that is unreachable: `online_image::update()` early-returns while `is_decoding()`, ticks are 1h apart, and a ~2.7s fetch cannot overlap a ~30s refresh. **Adding a second trigger (a button, a deep-sleep wake) makes it reachable**, and `epaper_spi` keeps both `state_` and `is_idle_()` private, so YAML cannot check first. Any new trigger has to be serialized against this one.
- **A 304 must not redraw.** `online_image` sends `If-None-Match`/`If-Modified-Since` and fires `on_download_finished` on *both* paths — `cached=true` for a 304, `false` after a real decode. The `!cached` guard is what stops the device burning a 30s refresh on an unchanged photo. Note ESPHome logs a 304 as `[E] HTTP Request failed ... Code: 304` and briefly sets an error flag; that is cosmetic, and it is the healthy steady state, not a fault.
- **`watchdog_timeout` on `http_request` is mandatory, not tuning.** The IDF request blocks the main loop, and the ESP32 task watchdog panics at 5s. With it unset — the default — an unreachable server **reboot-loops the device**: measured 6 fetch attempts, 6 watchdog panics, 6 reboots, heading for safe mode, with `on_error` never reached. `http_request` does wrap the request in a `WatchdogManager`, but `watchdog_timeout_` defaults to `0` and `WatchdogManager` returns early on zero, so the protection is entirely inert until configured. Set `watchdog_timeout` **above** `timeout` so the request always gives up first. Verified after the fix: 3 consecutive failures, each at exactly 10.03s, `on_error` firing, zero reboots.
- **The request timeout is 10s, not 30s.** A real fetch is ~2.7s, so 10s is ~4x margin while still failing fast. Note the observed server-down case was a connection *timeout*, not a refusal, so the device blocks for the whole timeout — under deep sleep that is radio-on time added to every failed wake.
- **Liveness is a timestamp, not a boolean.** `last_fetch_ok` advances on every successful fetch *including 304s*. A DNS failure and a dead server are the same silent event from the device's side, so a timestamp that stops moving is the only available signal; a success/failure flag would never be published by a device that has died. A moving timestamp with an unchanged panel means the server simply has not rotated the photo.
- **Timezone is set explicitly** to `Europe/Stockholm`, matching einkframe. Left unset, ESPHome resolves it via `tzlocal` at *compile* time, and the build runs in the esphome container where TZ is UTC — so UTC is baked into the binary. That is harmless for `last_fetch_ok`, which is deliberately stamped in UTC with a `Z` suffix so HA reads it as an absolute instant, but deep sleep must wake on a local-time schedule and Stockholm has DST.
- **The clock is SNTP, deliberately not the `homeassistant` platform.** HA being unreachable is one of the failure modes `last_fetch_ok` exists to reveal, so the clock must not depend on it. With the HA platform the device sat at `1970-01-01` indefinitely because no HA client had adopted it, and the timestamp silently never published.
- **The bezel hides 0–1 px, so no inset is needed** — measured in task 02 with a concentric-ring probe, superseding an earlier ~1–2 px eyeball estimate. The pale margin around the image is the panel's *non-addressable border region*, outside the 800×480 matrix and driven by the controller's border waveform, which is why it changes colour mid-refresh. It is not lost image area, and not configurable: ESPHome's `spectra_e6` init hardcodes `(0x50, 0x3F)` with no YAML option.
- The 1h cadence is a placeholder until deep sleep lands.

## Flashing

The default `esphome` compose service has **no device access** — it can only do OTA. A board that has never run ESPHome has no OTA to talk to, so the first upload needs the privileged `esphome-usb` service:

```bash
make upload USB=1 CONFIG=photoframe.yaml DEVICE=/dev/ttyACM0
```

Afterwards plain `make upload` over OTA works. The XIAO enumerates as native USB JTAG/serial (`303a:1001`) at `/dev/ttyACM0` — no CP210x driver involved, and it resets into the bootloader by itself.

## Open Items

- **HA adoption is deliberately deferred until deep sleep lands.** The device publishes
  `last_fetch_ok` but has not been added to Home Assistant, so the reporting path is
  unverified end to end. This is sequencing, not an oversight: adopting it now would
  register it as an always-on device, and once it starts sleeping through most of the day
  HA would read that as a permanently unavailable entity. Adopt it after the wake
  schedule exists, so it is registered as a deep-sleep device from the start.
- **SNTP sync races the boot fetch — handled at boot, still open for deep sleep.** Sync lands ~0–30s after boot; observed landing both before and after the first fetch across reboots. `on_boot` now waits for `utcnow().is_valid()` before fetching, so the first stamp is always valid. Under deep sleep that wait becomes radio-on time on every wake, so the wake sequence should stamp from `on_time_sync` instead of blocking on it.
- **Upstream bug: a mid-body connection close wedges `online_image` permanently.** `HttpContainer::read()` documents `0` as *"no data available yet OR all content read"* and tells callers to use `http_read_loop_result()` rather than inspect the return value. `online_image::loop()` inspects it directly as `if (len > 0) {…} else if (len < 0) {…}`, so `0` is treated as neither progress nor error. On ESP-IDF with a known `Content-Length`, a server that closes mid-body makes `esp_http_client_read()` return `0` indefinitely: the component busy-spins with `is_decoding()` true, never calls `disable_loop()`, never fires `on_error`, and every later `update()` bails with "Image already being updated." The panel freezes while the log claims nothing is wrong. Fix belongs upstream — consult `is_read_complete()` on the `len == 0` path. Not worked around here: **deep sleep mitigates it for free**, since every wake power-cycles the state, so a wedge lasts one cycle.
- **`online_image` blocks the main loop** for ~1.4s during fetch/decode, and the display for ~0.75s, tripping ESPHome's 50ms component warning. Harmless now; relevant if deep-sleep timing gets tight.
- **Color conversion is a cube-corner classifier, not nearest-color.** `epaper_spi/colorconv.h` forces near-greys (`max-min < 50`) to pure black or white and thresholds each channel at 128; cyan folds to green, magenta to red. The server must dither to exactly the six pure RGB primaries, or the panel reinterprets rather than approximates.
- **Deep sleep + battery monitoring.** Cadence is decided — once daily, aligned just after the server rotates. A full refresh measures **~29.8 s** with a real photo (~31 s was the pre-rotation task 01 figure) and is by far the biggest power draw, so refresh cadence and battery life are tightly coupled. einkframe's `voltage_to_battery_percent` and slot-based wake schedule are the obvious things to reuse.

## Testing

No native tests — there is no pure-logic C++ module yet. `make test` remains einkframe-only.
