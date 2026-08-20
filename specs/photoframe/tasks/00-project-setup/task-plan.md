# 00 — Project Setup

## Goal

Scaffold `photoframe` as the third project in the repo (alongside `einkframe`, `jukebox`): a color e-paper photo frame that fetches a pre-rendered image over HTTP and displays it.

## Decisions

- **Name**: `photoframe`. Entry point `config/photoframe.yaml`, packages under `config/packages/photoframe/`, specs under `specs/photoframe/`.
- **Board**: Seeed XIAO ePaper Display Board (EE0x family) + XIAO ESP32-S3 Plus. ESP-IDF framework, `esp32-s3-devkitc-1` variant. PSRAM enabled explicitly (`mode: octal`, `speed: 80MHz`).
- **Panel**: Waveshare 7.3" Spectra 6 (E6), 800×480, on the EE04's **50-pin** FPC (jumper set accordingly).
- **Display component**: mainline `epaper_spi` (ESPHome ≥ 2026.8.0), **not** `waveshare_epaper`. Model `Spectra-E6` — a controller-level model, so width/height and all pins are given explicitly.
- **Power**: 2000mAh Li-ion on the onboard JST 2.0 connector, same cell as einkframe. Deep sleep + battery monitoring deferred to task 01.
- **Pinout is already proven in this repo**: the EE0x driver board is pin-identical to the TRMNL board einkframe runs on — CLK=GPIO7, MOSI=GPIO9, CS=GPIO44, DC=GPIO10, RST=GPIO38, BUSY=GPIO4, battery ADC on GPIO1 gated by GPIO6. Reuse those directly, no guessing. Board also exposes 3 user buttons: KEY0=GPIO2, KEY1=GPIO3, KEY2=GPIO5 (active low).
- **Image source**: HTTP URL via ESPHome `online_image`. The ESP does no photo selection — it fetches one fixed URL. Whatever serves that URL (HA automation, script, or static endpoint) owns picking, cropping, and palette-mapping the photo. Out of scope for this task.
- **Refresh cycle**: same shape as einkframe — wake, fetch, draw, deep sleep. Schedule left generous (once or twice a day) because color refresh is slow and expensive.
- **Reuse**: `base.package.yaml` unchanged (wifi/OTA/API/logging via substitutions). New secrets keys `photoframe_home_assistant_api_encryption_key`, `photoframe_ota_password`.
- **No custom C++**: nothing to extract yet. `test/` and the `make test` target stay einkframe-only.

## Constraints

- **PSRAM required.** A 7.3" (800×480) 6-color buffer at 4bpp ≈ 192 KB; the 13.3" (1600×1200) is ≈ 960 KB. Add the decoded `online_image` buffer on top. Neither fits in internal RAM.
- **Color refresh is slow and current-hungry** (tens of seconds). Battery sizing and refresh frequency are coupled — assume mains or a large cell until measured.
- **Connector matters**: on the EE04 the 24-pin FPC carries 1.54"–7.5" panels, the 50-pin FPC carries the 7.3" Spectra 6. Selected by jumper. EE05 is 24-pin only, so it cannot drive the 7.3" Spectra 6.
- **Known upstream risk**: ESPHome issue [#12322](https://github.com/esphome/esphome/issues/12322) reports a bootloop with the Waveshare 7.3" E6 under `epaper_spi`. Check status before committing to that panel.

## Scope of this task

1. `config/photoframe.yaml` — substitutions, esphome block, packages include
2. `config/packages/photoframe/display.package.yaml` — SPI pins, waveshare_epaper, online_image, draw lambda
3. `specs/photoframe/ARCHITECTURE.md` — hardware, file structure, refresh cycle, open items
4. README + CLAUDE.md updated for a third project
5. `make validate CONFIG=photoframe.yaml` passes

## Out of scope

- The image server / HA side of the pipeline
- Battery monitoring and deep sleep tuning (follow-up task, needs the panel in hand)
- Any custom C++ module

## Open questions

- **Which panel?** The driver board is just a driver board. Panel choice sets resolution, color depth, `epaper_spi` model string, and whether photos are even viable (a mono panel means 1-bit dithered photos).
- **Which EE0x board exactly** — EE04 (24-pin + 50-pin) or EE05 (24-pin only). Only EE04 can drive the 7.3" Spectra 6.
- Battery or mains? Decides whether deep sleep is the next task.
