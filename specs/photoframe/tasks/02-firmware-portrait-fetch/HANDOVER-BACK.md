# Handover back → photoframe server repo

Device-side measurements from `specs/photoframe/tasks/02-firmware-portrait-fetch/` in the
esphome repo. Closes the last three open items in that repo's ARCHITECTURE, plus the
safe-area question and the palette the dither needs.

Hardware: XIAO ESP32-S3 Plus, 8MB PSRAM, ESP-IDF, ESPHome 2026.8.0. Panel: Waveshare
7.3" Spectra 6 (E6) via mainline `epaper_spi`, `rotation: 90°`, `online_image` at
`type: RGB565`, no `resize:`.

## The three measurements

Measured against the real 121197-byte 480×800 truecolour PNG served at
`photoframe.internal.pippinn.me:8088/current.png`, over four separate boots.

| | |
|---|---|
| Fetch + decode | **2672–2701 ms** (four boots, very stable) |
| Framebuffer | **768 000 bytes**, exactly as predicted for RGB565 480×800 |
| PSRAM resident after decode | **770 052 B** (~752 KiB) — buffer plus ~2 KB overhead |
| PSRAM peak, low-water mark | **814 088 B** (~795 KiB) |
| Transient decode scratch above the framebuffer | **~44 KB** |
| PSRAM free at the worst moment | **6.98 MiB of 8 MiB — ~89% still free** |
| Internal RAM peak | **~21 KB** above baseline |
| Full refresh, real photo | **29 818 / 29 821 / 29 830 ms** |

**RAM is a non-issue, decisively. Do not build the indexed PNG.** That fallback existed
only in case RGB565 would not fit; peak usage is 795 KB against 8 MB with ~7 MB spare.
There is no memory argument for a third-the-size format. If you ever want it for transfer
time, note that fetch+decode is under 3 s against a 30 s panel refresh — it would shave
under 2% off the wake budget. That decision can stay closed.

**"Decode time" cannot be separated from download on-device.** pngle decodes streaming as
bytes arrive, so the 2672 ms is fetch *and* decode combined. It is the closest measurable
quantity; treat it as an upper bound on decode.

## Safe area: there is none. Use the full 480×800

The bezel was measured with a probe of concentric 2 px rings. **Inset is 0–1 px** — the
outermost drawn pixel row is visible on all four edges. The earlier "~1–2 px under the
bezel" figure was an eyeball estimate and is withdrawn. Render to the full 480×800 with
no inset and no border.

The pale margin visible around the image is **not** lost pixels. It is the panel's
non-addressable border region, outside the 800×480 matrix, driven by the controller's
border waveform — which is why it changes colour during a refresh. It settles light and
acts as an accidental mat. It is not addressable and not configurable from ESPHome.

## Measured palette — this is the important part

The panel is **much** less saturated than the idealised Spectra 6 values circulating
online (e.g. yellow as `#FFF338`). Those are vendor swatches. These are photometric
measurements of this panel, relative to its own white, flat-fielded and validated:

| band | linear R G B | Y | sRGB | hex |
|---|---|---|---|---|
| black | 0.133 0.117 0.127 | 0.121 | (102, 96,100) | `#666064` |
| white | 1.000 1.000 1.000 | 1.000 | (255,255,255) | `#FFFFFF` |
| red | 0.379 0.127 0.096 | 0.178 | (166,100, 87) | `#A66457` |
| yellow | 1.010 0.827 0.317 | 0.829 | (255,235,153) | `#FFEB99` |
| blue | 0.271 0.340 0.562 | 0.341 | (142,158,198) | `#8E9EC6` |
| green | 0.219 0.286 0.227 | 0.267 | (129,146,131) | `#819283` |

Three consequences for the dither, in priority order:

1. **Contrast is 8.3:1, total.** The source photo must be tone-compressed before
   dithering or everything below ~12% luminance collapses into the single black primary.
2. **There is a hole in the tone scale between Y 0.341 (blue) and Y 0.829 (yellow).** No
   primary lands in the upper midtones, so that whole range must be dithered from
   blue/green against yellow/white. It will be the noisiest region of any image and is
   where tuning effort pays off.
3. **Blue is lighter than green and red** (0.341 vs 0.267 and 0.178) — the opposite of
   the usual assumption, and it inverts what you would expect when substituting them.

Full method and uncertainties in `panel-palette.md`; the bezel and rotation method in
`bezel-and-rotation.md`.

## Server-side things worth knowing

- **Everything the server guarantees was verified independently** before any firmware was
  written: 480×800, 8-bit truecolour, non-interlaced, no alpha, no palette chunk, exactly
  six distinct colours, 121197 bytes. All correct.
- **Non-interlaced matters.** pngle cannot decode interlaced PNG. If the encoder ever
  gains an Adam7 option, do not enable it.
- **The weak ETag (`W/"…"`) is fine.** `online_image` sends `If-None-Match` and
  `If-Modified-Since` and handles the 304 correctly.
- **304 works end to end.** Confirmed over repeated ticks: the device fetches, gets 304,
  allocates nothing, and does not redraw the panel. Keep serving 304s — each one avoided
  saves a 30 s refresh, which is the device's single largest power cost.
- **Nothing on the device needs the photo to change to prove liveness.** The device
  reports a `last_fetch_ok` timestamp that advances on 304s too, so a stalled server and
  an unrotated photo are distinguishable from the HA side.

## One request

Daily rotation (your task 04) is now the blocker for exercising the interesting path.
Everything on the device works, but with a fixed `current.png` the only code path that
ever runs after boot is the 304 branch. A photo that actually changes is needed to
confirm a *scheduled* redraw, as opposed to the boot redraw verified here.
