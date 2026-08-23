# Architecture

## Overview

ESPHome config for **photoframe** — a battery-powered 7.3" color e-paper photo frame, **hung portrait**. Fetches a pre-rendered PNG over HTTP once a day and draws it. The device does no photo selection and no image processing: whatever serves the URL owns picking, cropping, and palette-mapping.

The panel is natively 800×480 landscape; rotated 90° for portrait mounting the drawing surface is **480 wide × 800 tall**, which is what the served PNG must be.

Distinct from [einkframe](../einkframe/ARCHITECTURE.md), which is a mono *dashboard* frame. Same driver-board family, different panel and different job.

## File Structure

```
config/
  photoframe.yaml                     # Entry point: substitutions, PSRAM, flash, on_boot
  packages/
    base.package.yaml                 # Shared: wifi, OTA, API, logging
    photoframe/
      display.package.yaml            # SPI, http_request, online_image, epaper_spi, refresh script
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
- **Busy pin is active-low**: `inverted: true` with a pullup. Verified on hardware. Getting this wrong fails silently — the driver never waits, logs a ~1s "successful" refresh, and leaves the panel blank. A real full refresh takes **~31 s**; anything much faster means it isn't actually driving the panel.
- **Requires ESPHome ≥ 2026.8.0** — `epaper_spi` is not in earlier releases. `compose.yaml` tracks `:latest`, so a stale local image will fail validation with confusing schema errors. `docker compose pull esphome` fixes it.
- **PSRAM is mandatory**, declared as `mode: octal, speed: 80MHz`. The 800×480 6-color frame buffer (~192KB at 4bpp) plus the decoded `online_image` buffer do not fit in internal RAM. Confirmed at boot: `Available: YES, Size: 8192 KB`.
- Refresh path: `refresh_photo` script waits for wifi → `component.update: photo` downloads the PNG → `on_download_finished` fires `component.update: epaper` → lambda draws it. Both the image and the display are set to `update_interval: never`; nothing refreshes on a timer by itself.
- Currently a 1h `interval:` drives refreshes. That is a placeholder until deep sleep lands.

## Flashing

The default `esphome` compose service has **no device access** — it can only do OTA. A board that has never run ESPHome has no OTA to talk to, so the first upload needs the privileged `esphome-usb` service:

```bash
make upload USB=1 CONFIG=photoframe.yaml DEVICE=/dev/ttyACM0
```

Afterwards plain `make upload` over OTA works. The XIAO enumerates as native USB JTAG/serial (`303a:1001`) at `/dev/ttyACM0` — no CP210x driver involved, and it resets into the bootloader by itself.

## Open Items

- **Image pipeline not built.** `photo_url` points at a placeholder, the fetch block is commented out, and the bring-up test pattern is what is flashed. 
- **Color conversion is a cube-corner classifier, not nearest-color.** `epaper_spi/colorconv.h` forces near-greys (`max-min < 50`) to pure black or white and thresholds each channel at 128; cyan folds to green, magenta to red. The server must dither to exactly the six pure RGB primaries, or the panel reinterprets rather than approximates.
- **~1–2 px at the extreme edge** sits under the panel bezel. Keep content inset.
- **Deep sleep + battery monitoring** Cadence is decided — once daily, aligned just after the server rotates. A full refresh measures **~31 s** on this panel and is by far the biggest power draw, so refresh cadence and battery life are tightly coupled. einkframe's `voltage_to_battery_percent` and slot-based wake schedule are the obvious things to reuse.

## Testing

No native tests — there is no pure-logic C++ module yet. `make test` remains einkframe-only.
