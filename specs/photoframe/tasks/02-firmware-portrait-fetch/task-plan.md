# 02 — Firmware Portrait + Photo Fetch (plan)

Turn on the image pipeline task 01 left commented out: rotate to portrait, fetch the
real 480×800 PNG, draw it, report liveness to HA, and measure the three things only the
device can measure.

Numbered 02 here (sequential in this repo). The server-side repo calls the same work
**03**, and its later task numbers are referenced below — they are that repo's
numbering, not this one's.

## The server side

Everything this task needs to know about the thing on the other end of the URL.

**Endpoint** — plain HTTP, no TLS, no auth, LAN only:

```
http://photoframe.internal.pippinn.me:8088/current.png
```

The name is a plain A record on both PiHoles, not a Traefik route, so nothing is
proxied. It resolves to the **Pi (`192.168.86.26`)**, not the NAS — the renderer
`SIGILL`s on the NAS CPU. `http://192.168.86.26:8088/current.png` is the same file by
IP, which makes it the one-step bisect when a fetch fails: **IP works, name doesn't →
DNS; neither works → server.**

`*.internal.pippinn.me` has a wildcard pointing at the NAS, and the photoframe record is
a deliberate override of it. If that override is ever lost the device resolves the NAS
(`192.168.86.17`), gets connection-refused on 8088, and silently keeps yesterday's
photo. Check the name before flashing:
`dig +short photoframe.internal.pippinn.me @192.168.86.26` — `.26` is right, `.17`
means the override is gone.

**What the server guarantees**

| | |
|---|---|
| Dimensions | exactly 480 × 800, portrait, upright |
| Colours | exactly the six cube-corner values, asserted end-to-end in the server's CI |
| Encoding | truecolour PNG, 8-bit RGB, no alpha, no palette chunk, max compression |
| Size | ~121 KB for the current photo; largest plausible render ~157 KB |
| Headers | `ETag` and `Last-Modified`; `304` on `If-None-Match` / `If-Modified-Since` |
| Availability | while the container is up; nothing else fetches or rewrites the file |

**Two things that will look like firmware bugs and are not**

- **The photo never changes.** There is no photo source and no scheduler on the server
  yet — `current.png` is one fixed stock portrait, so `ETag` and `Last-Modified` never
  move. A device that stays awake gets a `304` forever and, with the `!cached` guard,
  never redraws. That is the guard working. Daily rotation is the server's task 04.
- **The panel will look like coloured noise.** The dither is at neutral defaults;
  tuning is the server's task 05, after this task lands. Do not read a rough-looking
  panel as a firmware fault — judge the **geometry** instead: right way up, no scaling,
  no colour the panel cannot show. The image is also a freely-licensed stock portrait of
  a stranger, used as the test fixture; it goes away with the server's task 04.

## Verified before planning

Checked, not assumed — facts as of 2026-08-23:

- `photoframe.internal.pippinn.me` resolves to `192.168.86.26` from this LAN, both via
  the PiHole directly and via the default resolver. The wildcard override is intact.
- The served PNG is **480×800, 8-bit truecolour RGB, non-interlaced, no alpha, no
  palette, 121197 bytes**, and contains **exactly 6 distinct colours** — the six cube
  corners. Downloaded, decoded, and colour-counted locally. Every guarantee above holds.
- `ETag` is **weak** (`W/"…"`). Valid for `If-None-Match`; no impact.
- ESPHome in the local Docker image is **2026.8.0**. `online_image` sends
  `If-None-Match` and `If-Modified-Since`, and fires `on_download_finished` on **both**
  paths — `cached=true` on a 304, `cached=false` after a real decode. Read from
  `online_image.cpp`, not the docs.
- `epaper_spi` accepts `rotation` (`has_hardware_rotation=True`).

## Scope

In: rotation, drop `resize:`, RGB565, the `!cached` redraw guard, one HA liveness
sensor, bezel measurement, the three measurements, and the write-ups.

Out: deep sleep and battery monitoring. Both depend on the refresh-time and power
numbers this task produces, and deep sleep makes the fetch path much harder to debug —
the device is gone before you can read the log.

## Decisions

**Rotation** — `rotation: 90°` on the `epaper_spi` block, so the drawing surface is
480 wide × 800 tall and the PNG lands at `it.image(0, 0, photo)` with no scaling.
90 vs 270 is a coin flip from the config alone; the bezel probe below settles it.

**No `resize:`** — the served PNG is already exactly the drawing surface. Any resize
would resample six pure colours into intermediate values the panel's classifier then
reinterprets rather than approximates.

**Buffer type** — `RGB565`, 768 KB of the 8 MB PSRAM. Rounding to 5/6/5 bits cannot
move any of the six cube-corner colours across `colorconv`'s 128 threshold, so the
palette survives exactly. `RGB24` would cost 380 KB more and buy nothing the panel's
classifier can see.

**URL** — `photo_url: !secret photoframe_photo_url`. This repo is public and currently
holds no hostnames, only credentials. Keeping it in `secrets.yaml` also makes the
DNS-vs-server bisect above a non-commit.

**Trigger** — `on_boot` runs `refresh_photo`; the 1h `interval` stays as the
placeholder ARCHITECTURE already documents. The image cache is RAM-only, so **reboot is
the only way to re-run a decode** — hourly ticks can only ever return 304 until the
server starts rotating daily. Those 304s are near-free and are what keeps the liveness
timestamp moving.

**HA reporting** — one `text_sensor`, `last_fetch_ok`, `device_class: timestamp`,
ISO8601, stamped in `on_download_finished` regardless of `cached`. A DNS failure and a
dead server are the same event from the device's side and both are silent by design, so
a timestamp that stops moving is the only available signal. Deliberately not publishing
success/failure booleans: a device that dies never publishes a failure either.

**Redraw guard** — `component.update: epaper` only fires when `!cached`. A 31 s refresh
for an unchanged photo is the single most expensive pointless thing this device could do.

**Font** — the `font_label` block goes with the test pattern. Nothing draws text
afterwards, and the design forbids ever drawing an error on the panel. It costs flash
and a Google Fonts fetch on every build; `git show` recovers it if a later task wants one.

**Failure behaviour** — server unreachable leaves the previous photo on the panel, logs,
and draws nothing. A wall frame showing a stack trace is worse than one showing
yesterday. E-paper holds its image with no power and `component.update: epaper` is
reachable only from `on_download_finished`, so doing nothing is not a special case — it
is the default.

## Bezel measurement

ARCHITECTURE claims ~1–2 px at the extreme edge sits under the bezel. That is an
eyeball estimate from the task 01 photo and looked low. Measure it properly: it is the
safe-area figure the server side needs.

A **concentric ring ruler** — roughly ten 2 px rings from the outermost pixel inward,
cycling red / green / blue / yellow / black, drawn **with `rotation: 90°` already
applied**, plus an asymmetric corner marker. The first fully-visible ring colour gives
the inset per edge in 2 px units, and the marker's position in the photo settles 90 vs
270. One flash and one macro photograph answer both questions.

The probe replaces the display lambda in `display.package.yaml` temporarily and is
reverted, not kept. No new entry point, no duplicated pinout.

## Sequence

Steps 0 and 1 are gates. Do not flash past them.

0. **Photograph the six-band pattern now on the panel.** The server's task 03 calibrates
   its dither against those six sampled RGB values — they are load-bearing, not
   cosmetic, and recovering them costs a re-flash of bring-up. Nothing else starts until
   the photograph exists.
1. **Bezel probe.** Flash the ring ruler, photograph, record the per-edge inset and the
   rotation direction, revert the lambda.
2. **Fetch path.** Uncomment and correct `http_request` / `online_image` / `script` /
   `interval`, drop `resize:`, apply the confirmed rotation, wire the `!cached` guard,
   add `last_fetch_ok`, delete the test pattern and font, point `photo_url` at the
   secret. Flash and confirm a real photo, right way up, unscaled.
3. **Measure.** Temporary lambda logging of free PSRAM and heap before the fetch, in
   `on_download_finished`, and after the draw; decode and refresh durations from log
   timestamps. Reboot a few times for a spread. Instrumentation comes out afterwards.
4. **Failure test.** Stop the server container on the Pi and reboot the device — truer to
   the real failure (connection refused on a name that resolves) than a bad URL. Confirm:
   `on_error` logs, `last_fetch_ok` stalls, panel unchanged, no error drawn.

## Escalation

If the decode fails or PSRAM will not stretch, say so plainly and **do not work around
it on the device**. That finding reopens a server-side decision — a hand-built indexed
PNG at roughly a third the size, rejected so far only because ESPHome's decoder support
for it is unverified. Cheaper to fix once on the server than to carry a workaround in
firmware forever.

## Deliverables

- `task-summary.md` — actions, lessons, and the measured numbers.
- `HANDOVER-BACK.md` — aimed at the server repo's agent, closing the last three open
  items in its ARCHITECTURE: PNG decode time on the ESP32-S3 for a ~121 KB truecolour
  480×800 PNG; peak RAM during decode and how much headroom is left, given RGB565 costs
  768 KB of PSRAM for the framebuffer and the decode sits on top of that; and
  full-refresh draw time against a real photo rather than the test pattern (~31 s
  expected from task 01). Plus the measured bezel safe area and a verdict on the
  indexed-PNG question.
- `specs/photoframe/ARCHITECTURE.md` — promote decode time, RAM headroom, real-photo
  refresh time, rotation, and the corrected bezel inset from open questions to confirmed
  facts, the way task 01 did with busy polarity. Close the "image pipeline not built"
  open item.

## Success criteria

- A real photo, upright, unscaled, filling the panel.
- Rebooting redraws; staying awake does not.
- `last_fetch_ok` advances hourly in HA.
- Server down → panel keeps the old photo, timestamp stalls, nothing drawn.
- All three measurements recorded, plus the bezel inset per edge.
