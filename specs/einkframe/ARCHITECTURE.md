# Architecture

## Overview

ESPHome config project for **eInkFrame** — ESP32-S3 + 7.5" Waveshare e-paper display. Shows weather (SMHI), battery, Icelandic word of the day w/ Swedish translations. All data from Home Assistant.

## File Structure

```
config/
  einkframe.yaml                    # Entry point: display lambda, scripts, SPI pins
  secrets.yaml                      # WiFi creds (gitignored)
  packages/
    base.package.yaml               # Shared: wifi, OTA, API, logging
    einkframe/
      sensors.package.yaml          # HA text_sensor + sensor definitions
      fonts.package.yaml            # Google Fonts (Inter) + MDI icons
      text_utils/                   # Custom C++ via esphome.includes
        text_utils.h
        text_utils.cpp
      einkframe_utils/              # Pure logic extracted from YAML lambdas
        einkframe_utils.h
        einkframe_utils.cpp
      renderer_utils/               # Display panel draw functions
        renderer_utils.h
        renderer_utils.cpp
test/
  doctest.h                         # Vendored single-header test framework
  test_einkframe_utils.cpp          # Native unit tests (run via `make test`)
```

## How It Fits Together

- `einkframe.yaml` uses ESPHome `packages:` for modular YAML and `esphome.includes` for custom C++
- Display rendering = a thin C++ lambda that builds per-panel state structs and calls `renderer_utils` draw functions
- Refresh cycle: scripted sequence — enable ADC → read battery → wait for HA sensors → update display. Triggered on boot, then deep sleeps until next slot
- Deep sleep schedule: 07, 11, 15, 19 (12h overnight gap). Sleep duration calculated dynamically to hit next slot
- Dev Mode: `input_boolean.einkframe_dev_mode` in HA controls deep sleep. ESP subscribes via `binary_sensor` + homeassistant platform and calls `deep_sleep.prevent`/`allow` on state change. HA owns the state so it's toggleable while frame sleeps. Reset button wakes frame to pick up new state. When on, refreshes every 15 min

## Hardware

Based on the [TRMNL 7.5" OG DIY Kit](https://www.seeedstudio.com/TRMNL-7-5-Inch-OG-DIY-Kit-p-6481.html). PCB has buck converter for ESP32 power, onboard voltage divider for battery ADC.

- **Board**: ESP32-S3 (esp32-s3-devkitc-1), Arduino framework
- **Display**: Waveshare 7.5" v2 e-paper, SPI (CLK=GPIO7, MOSI=GPIO9, CS=GPIO44, DC=GPIO10, RST=GPIO38, BUSY=GPIO4 inverted)
- **Battery**: 2000mAh Li-ion (3.0–4.2V nominal), ADC on GPIO1 with voltage divider (x2), gated by GPIO6
- **Usable display area**: ~(55,45) to (739,479)

## Custom C++

Three modules, all included via `esphome.includes`. No separate build — ESPHome compiles them into the firmware. Signatures must match lambda usage.

- **text_utils** — `get_text_width()` wraps ESPHome's `font::Font::measure()`. `wrap_text()` is now a thin adapter that forwards to `einkframe_utils::wrap_text_pure` with a Font-backed measurer, so the wrapping algorithm itself is unit-testable.
- **einkframe_utils** — Pure functions (no ESPHome deps). Currently: `minutes_to_next_slot(hour, minute)`, `voltage_to_battery_percent(voltage)`, `weather_icon(state)` (HA weather state → MDI codepoint), `ui_icon(key)` (internal UI key → MDI codepoint), and `wrap_text_pure(text, max_width, measurer)` — the font-agnostic wrapping algorithm. Add further extractable pure logic here.
- **renderer_utils** — Display orchestration. Exports three panel draw functions (`draw_title`, `draw_info_column`, `draw_word_of_day`) and their per-panel state structs (`TitleState`, `InfoColumnState`, `WordOfDayState`, plus `ExamplePair`). Layout constants and small drawing helpers (wrapped-line blocks, info rows) live `static` in the `.cpp`. Couples to `esphome::display::Display` and `esphome::font::Font` — not unit-testable in the doctest harness; verified by hardware.

Cross-module includes use relative paths (e.g. `text_utils` imports `../einkframe_utils/einkframe_utils.h`), because each `esphome.includes` directory is added to the compiler include path as its own root, not as a sibling set.

## Testing

`make test` compiles [test/test_einkframe_utils.cpp](../test/test_einkframe_utils.cpp) natively with g++ (C++17) and runs the doctest suite. The `einkframe_utils` module is fully tested — including the wrapping algorithm via `wrap_text_pure` with a fake measurer. The `text_utils` Font adapter and all of `renderer_utils` remain ESPHome-bound and need hardware-in-the-loop testing.
