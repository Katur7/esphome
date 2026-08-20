# esphome

Personal ESPHome configs for my ESP32 projects. Most run against my Home Assistant instance and share a base package for WiFi, OTA, API, and logging.

## Projects

### eInkFrame

Battery-powered 7.5" e-paper frame. Shows Icelandic word of the day with Swedish translations, weather (SMHI), and battery state. ESP32-S3 + Waveshare 7.5" v2, built on the [TRMNL 7.5" OG DIY Kit](https://www.seeedstudio.com/TRMNL-7-5-Inch-OG-DIY-Kit-p-6481.html).

Wakes on a schedule (07/11/15/19), refreshes the display, deep sleeps. A `dev_mode` toggle in Home Assistant keeps it awake for development and refreshes every 15 min.

Custom C++ helpers (`text_utils`, `einkframe_utils`, `renderer_utils`) are pulled in via `esphome.includes` and compiled into the firmware. The pure-logic module has native unit tests via doctest.

Full architecture: [specs/einkframe/ARCHITECTURE.md](specs/einkframe/ARCHITECTURE.md).

### Photoframe

Battery-powered 7.3" color e-paper photo frame. Fetches a pre-rendered 800×480 PNG over HTTP and draws it — the device does no photo selection, whatever serves the URL owns that.

Seeed XIAO ePaper Display Board EE04 + XIAO ESP32-S3 Plus, driving a Waveshare 7.3" Spectra 6 panel on the 50-pin connector. Uses the `epaper_spi` component, so it **needs ESPHome ≥ 2026.8.0** — run `docker compose pull esphome` if validation fails with odd schema errors.

Full architecture: [specs/photoframe/ARCHITECTURE.md](specs/photoframe/ARCHITECTURE.md).

### Jukebox

ESP32-C6 with an RC522 NFC reader. Scans tags and logs them — currently a starting point.

## Layout

- `config/` — ESPHome YAML entry points (`einkframe.yaml`, `photoframe.yaml`, `jukebox.yaml`), shared `packages/`, fonts, and `secrets.yaml` (gitignored)
- `specs/` — architecture docs and per-task plans/summaries
- `test/` — native doctest suite for the pure-logic C++ helpers

## Prerequisites

- Docker (ESPHome runs via `ghcr.io/esphome/esphome`)
- A Home Assistant instance the device can reach (both projects rely on the HA API)
- For eInkFrame: HA entities for weather, current date, UV index, expected rain, and the word-of-the-day sensors listed in [config/packages/einkframe/sensors.package.yaml](config/packages/einkframe/sensors.package.yaml). Also `input_boolean.einkframe_dev_mode` for the dev-mode toggle.

## Setup

Create `config/secrets.yaml`:

```yaml
wifi_ssid: "..."
wifi_password: "..."
ap_password: "..."
home_assistant_api_encryption_key: "..."
ota_password: "..."
jukebox_home_assistant_api_encryption_key: "..."
jukebox_ota_password: "..."
photoframe_home_assistant_api_encryption_key: "..."
photoframe_ota_password: "..."
```

## Commands

All targets default to `CONFIG=einkframe.yaml`. Pass `CONFIG=photoframe.yaml` or `CONFIG=jukebox.yaml` to target another project. `DEVICE=...` overrides the upload/log device.

```bash
make compile                    # compile
make upload                     # OTA upload
make logs                       # stream logs
make run                        # compile + upload + logs
make validate                   # validate config
make dashboard                  # ESPHome web dashboard on :6052
make clean                      # clean build artifacts
make test                       # native C++ unit tests (einkframe_utils)

make run CONFIG=photoframe.yaml # build/upload photoframe instead
make run CONFIG=jukebox.yaml    # build/upload jukebox instead
make upload DEVICE=/dev/ttyUSB0 # force a specific port
```

## Workflow

Spec-driven. Each task lives under `specs/<project>/tasks/NN-name/` with a `task-plan.md` (decisions, agreed before implementing) and a `task-summary.md` (written after). See [CLAUDE.md](CLAUDE.md).

## License

MIT — see [LICENSE](LICENSE).
