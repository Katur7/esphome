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
```

## How It Fits Together

- `einkframe.yaml` uses ESPHome `packages:` for modular YAML and `esphome.includes` for custom C++
- Display rendering = single C++ lambda calling `text_utils` for measurement/wrapping
- Refresh cycle: scripted sequence — enable ADC → read battery → wait for HA sensors → update display. Triggered on boot, then deep sleeps until next slot
- Deep sleep schedule: 07, 11, 15, 19 (12h overnight gap). Sleep duration calculated dynamically to hit next slot
- Dev Mode: HA-exposed switch disables deep sleep for OTA/development. When on, refreshes every 15 min. Default off. Reset button wakes from sleep

## Hardware

Based on the [TRMNL 7.5" OG DIY Kit](https://www.seeedstudio.com/TRMNL-7-5-Inch-OG-DIY-Kit-p-6481.html). PCB has buck converter for ESP32 power, onboard voltage divider for battery ADC.

- **Board**: ESP32-S3 (esp32-s3-devkitc-1), Arduino framework
- **Display**: Waveshare 7.5" v2 e-paper, SPI (CLK=GPIO7, MOSI=GPIO9, CS=GPIO44, DC=GPIO10, RST=GPIO38, BUSY=GPIO4 inverted)
- **Battery**: 2000mAh Li-ion (3.0–4.2V nominal), ADC on GPIO1 with voltage divider (x2), gated by GPIO6
- **Usable display area**: ~(55,45) to (739,479)

## Custom C++ (text_utils)

`get_text_width()` and `wrap_text()` use ESPHome's `font::Font::measure()`. Called from display lambda for dynamic positioning and word-wrapping. No separate build — ESPHome compiles them in. Signatures must match lambda usage.
