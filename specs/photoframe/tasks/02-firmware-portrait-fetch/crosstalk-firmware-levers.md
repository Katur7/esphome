# Firmware levers for Spectra-6 line crosstalk — findings

Read-only investigation answering `handover.md`. Nothing changed. Sources: this repo's
config, the generated build tree (`config/.esphome/build/photoframe/src/`), the
`epaper_spi` sources in `ghcr.io/esphome/esphome:latest` (2026.8.0), and
`waveshareteam/e-Paper` master (`EPD_7in3e.c`, fetched 2026-09-01).

## Per lever

### 1. Display component / driver — exists, correct family, exact-panel init

`epaper_spi`, `model: 7.3in-Spectra-E6` → C++ class `EPaperSpectraE6`, a ~40-line
subclass of the generic 4bpp base. The Spectra-E6 family shares one init table; the
7.3" model entry adds 800×480 and `data_rate: 20MHz`
(`components/epaper_spi/models/spectra_e6.py`). Command set is the UC-family style with
the 0xAA "CMDH" unlock — the same controller Waveshare's 7.3" (E) code drives. The init
table is byte-identical to Waveshare's `EPD_7IN3E_Init`, so it is written for exactly
this panel, not a sibling.

### 2. Waveform/LUT — from panel OTP, already-correct, not configurable

PSR (cmd 0x00) = `5F 69`: REG bit 0 → controller loads the factory waveform from the
panel's own OTP. No LUT is hardcoded anywhere in the component. Nothing newer in vendor
master; ESPHome and Waveshare agree byte-for-byte. **No waveform knob exists to turn.**

### 3. Temperature — automatic, matches vendor, not configurable

Neither ESPHome nor the vendor sample touches any temperature command
(0x40/0x41/0xE0/0xE5). This controller family samples its internal temp sensor during
power-on (0x04) and selects the OTP waveform band itself. At 18–25 °C indoor it sits in
the nominal band. Already-correct in the sense of "identical to vendor reference";
there is no register the driver could set better.

### 4. Booster/VCOM/init registers — one real deviation found

Init (booster BTST1/2/3, VCOM/CDI 0x50=3F, T_VDCS 0x84=01, PLL, TCON, POFS, PWS) is
**byte-identical** to `EPD_7IN3E_Init`. But vendor `EPD_7IN3E_TurnOnDisplay` re-sends
**BTST2 (0x06 = 6F 1F 17 49) after power-on (0x04 + busy) and immediately before every
refresh (0x12)**. ESPHome's state machine goes 0x04 → busy → 0x12 with no 0x06 in
between (`epaper_spi_spectra_e6.cpp` power_on/refresh_screen). The bytes equal the init
values, so it only matters if power-on reloads that register from OTP — vendor code
bothers to do it every refresh. Booster drive strength is exactly the lever the working
theory points at (source-rail stiffness under max-switching load). **Not
YAML-configurable; a patched component copy under `external_components` is ~5 lines.**

Minor, likely irrelevant: reset timing differs (ESPHome 10 ms low pulse, then
busy-wait; vendor 2 ms low / 20 ms high / busy / +30 ms). `reset_duration` is
YAML-configurable if we ever care.

### 5. Refresh/update modes — only one exists

One full refresh: `0x12 0x00`. No partial mode, no fast/quality variants in either the
ESPHome driver or the vendor sample; `full_update_every` is forced to 1. Power-off
(0x02) + deep-sleep (0x07 A5) after every refresh matches vendor. **Nothing unused to
enable.**

### 6. Data clocking — configurable, long shot

20 MHz comes from the ESPHome model default; vendor RPi demo clocks slower. One-line
YAML experiment: `data_rate: 10MHz`. Note SPI corruption would flip pixel codes in
controller RAM (wrong-colored dots), not produce an analog per-line wash, so this
stays a long shot.

## Cable / power supply — asked separately

**Badly seated FPC as the root cause: unlikely.** A marginal 50-pin contact classically
gives fixed-position streaks, dead columns, or region-wide casts tied to physical pins,
varying with reseating and flex — not a content-gated, line-exact artifact identical on
every refresh.

**Power delivery as a contributor: plausible, and determinism does not rule it out.**
The panel rails (VDH/VDL/VGH/VGL, VCOM) come from boost converters on the EE04 board
fed by the XIAO's 3.3 V LDO. A line of dithered blue+yellow switches the source outputs
maximally → peak current on exactly that gate step → rail droop that corrupts the black
drive on the same line. Identical content → identical load → identical droop, so
"deterministic" is consistent with supply sag too. The slow ~400-line build-up also
reads like progressive rail/bulk-cap droop. Extra series resistance from a creased FPC
or half-latched connector on the supply pins produces the same signature.

Zero-code tests, in order:
1. Reseat the FPC (latch fully closed, ribbon not creased), re-run `crosstalk-probe.png`.
2. Known-good 5 V ≥2 A supply, short thick USB cable, no hub; repeat the probe.
3. If battery-capable: probe on battery vs USB. Amplitude change with power source ⇒
   delivery problem; no change ⇒ panel physics, content mitigation is the right path.

## Verdict table

| Lever | Status |
|---|---|
| Driver/model | exists; exact-panel init, vendor-matched |
| Waveform/LUT | OTP-loaded; already-correct; no knob |
| Temperature | automatic internal; matches vendor; no knob |
| Booster/init | **deviation: vendor re-sends 0x06 before every 0x12; ESPHome doesn't** — patchable |
| Refresh modes | single mode; nothing unused |
| SPI clock | 20 MHz; YAML `data_rate` knob; long shot |
| Cable/FPC | unlikely root cause; reseat is free |
| Power supply | plausible contributor; testable without code |

## If a panel session is bought, spend it in this order

1. Power/cable/FPC tests above (no code, same probe image).
2. `data_rate: 10MHz` in `display.package.yaml` (one line).
3. Patched `epaper_spi_spectra_e6.cpp` via `external_components`: send
   `0x06 = 6F 1F 17 49` in `power_on()` after 0x04, mirroring vendor `TurnOnDisplay`.
