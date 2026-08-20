# 00 — Project Setup (summary)

## Actions

1. `config/photoframe.yaml` — substitutions, PSRAM (octal/80MHz), 16MB flash, on_boot refresh
2. `config/packages/photoframe/display.package.yaml` — SPI, `http_request`, `online_image`, `epaper_spi` (Spectra-E6), refresh script, 1h placeholder interval
3. `specs/photoframe/ARCHITECTURE.md` — hardware, pinout table, refresh path, open items
4. Appended `photoframe_*` keys to `config/secrets.yaml` (generated values — replace the API key with HA's if pairing an existing device)
5. README: Photoframe section, secrets list, commands. CLAUDE.md: rewritten for a monorepo of three projects, stale `specs/ARCHITECTURE.md` and `specs/tasks/` paths fixed
6. `make validate CONFIG=photoframe.yaml` passes; einkframe and jukebox re-validated after the image upgrade

## Lessons

- **The driver board was already known.** The Seeed XIAO ePaper Display Board EE0x is the same board family as einkframe's TRMNL kit — pin-identical (CLK=7, MOSI=9, CS=44, DC=10, RST=38, BUSY=4, battery ADC=1 gated by 6). Checking the existing ARCHITECTURE.md before researching would have been faster than the web search.
- **Color e-paper is a different component.** `epaper_spi`, not `waveshare_epaper`. Mainline only since ESPHome 2026.8.0.
- **`compose.yaml` tracks `:latest` but Docker caches.** The local image was 2026.3.3, which failed with misleading schema errors (`'file' is a required option for [image]`) rather than "component not found" — the older release also had `online_image` shaped differently. `docker compose pull esphome` was the fix. Upgrading the shared image affects every project, so einkframe and jukebox were re-validated; both fine.
- **Deferred deliberately**: battery/deep sleep, the image server, and busy-pin polarity. The last two can't be settled without the hardware powered on.

## Architecture doc

Created fresh at `specs/photoframe/ARCHITECTURE.md`. einkframe's is untouched.
