# Task 08 — Design revamp (07a "Orð dagsins" port)

Port the 07a layout from the design handoff to the device renderer. The
design package in this folder (`DESIGN.md`, `README.md`, `preview/`) is the
source of truth — specifically the `VariantOrdA` component and `.v-ord-a`
CSS in `preview/variant-ord.jsx` / `preview/styles.css`.

## Goal

Replace the current three-column layout with the 07a folio layout:

- Folio header: small-caps `ORÐ DAGSINS · <date>` left, weather icon + temp right, hairline rule.
- Body, two columns: Íslenska peg → headword → word-class → Svenska peg → dotted translations · Dæmi peg → example list with hairlines between items.
- Bottom strip: 4 cells (Regn / UV-index / Väder / Batteri), hairline-separated, framed by a top rule.

Acceptance: render matches `preview/index.html` for the three scenarios — `regn` (short), `fjallganga` (medium), `sjálfsmorðshugleiðingar` (pressure / 28 translations).

## Scope

In:

- New panel functions in `renderer_utils` and matching state structs.
- Add new fonts to `fonts.package.yaml` (Newsreader, more Inter weights, JetBrains Mono).
- Wire updated lambda in `einkframe.yaml`.
- Helpers for the headword auto-fit, translation splitting + cap, and weather → Swedish label.

Out:

- 07b variant (skip — not in scope).
- New HA sensors. Optional `inflections` / `posLong` are nice-to-haves; if not exposed by HA, fall back to existing `pos` / hide the inflection line.

## Approach

Three new panel functions, replacing the existing three:

1. **`draw_folio_header`** — date + weather + temp + hairline rule.
2. **`draw_entry`** — two-column body: hero (peg, headword, word-class, optional inflection, peg, dotted translations) and examples (peg, list with hairlines).
3. **`draw_strip`** — four equal cells with hairlines and top rule.

Layout constants stay `static` in the `.cpp` (current pattern). Tune Y/X offsets visually on hardware after first port — the constants in DESIGN.md "Porting plan" are starting points, not gospel.

## Pure logic (testable in `einkframe_utils`)

Extract these so the doctest harness covers them:

- `split_translations(s)` — split on `,` or `·`, trim, drop empties.
- `cap_translations(items, max=15)` — return visible slice + overflow count for the `+N fler` tag.
- `fit_headword_size(text, min, max, measurer)` — binary-search font size given a width-measurer (mirrors the JS `useFitHeadword`). Falls back to wrap-at-floor when even `min` overflows.
- `weather_label(state)` — HA weather state → Swedish word (Newsreader cell on the strip).
- `pos_long(pos)` — extract the inside-parens word class from existing `pos` field.

Renderer code calls these; tests cover them headlessly.

## Renderer-bound work (verified on hardware)

- Headword: use `fit_headword_size` with a font-stack of pre-built Newsreader sizes (we can't dynamically resize a font — pick from a discrete ladder of sizes loaded at compile time, e.g. 36/44/52/60/68/74).
- Dotted translation list: render items with `·` separators, wrap at spaces only. When the item count exceeds the cap, append `+N fler` glyph. Stop early if the column would overrun the strip.
- Example list: hairline between adjacent items, IS in serif italic, SE in serif regular smaller.
- Hairlines vs rules: 1 px hairlines between cells / between examples; 3 px solid rules under header and above strip.

## Fonts

Add to `fonts.package.yaml` (Google Fonts via `gfonts://`):

- Newsreader 500 — headword, multiple sizes (size ladder above).
- Newsreader 400 — translations, examples (regular).
- Newsreader 400 italic — word class, IS examples.
- Inter 300 / 500 / 800 — folio header, peg labels, strip values, `+N fler`.
- JetBrains Mono 700 — only if peg labels need it (DESIGN.md says small-caps Inter is fine; default to Inter unless visual test says otherwise).

Existing Inter / MDI fonts stay (still used by strip + folio weather glyph).

## Data fields

No HA-side changes required. Renderer derives:

- `posLong` from existing `pos` via `pos_long()`.
- `inflections` — optional; hide line if empty/missing.
- `weatherLabel` — derived from existing `myWeather` via `weather_label()`.
- Strip values reuse existing sensors verbatim.

## Risks / open questions

- **Auto-fit on a discrete size ladder** vs. continuous: e-ink fonts are pre-baked at compile time, so we can't binary-search to an arbitrary px. Pick the largest ladder size that fits — good enough.
- **Glyph coverage**: Newsreader needs full Latin + Icelandic + Swedish diacritics. Verify glyphsets cover `þ ð æ ö ä å`.
- **MDI codepoints**: confirm any new icons used (`umbrella`, `uv`, `battery`, weather variants) are in the existing `font_mdi_medium` glyph list; add missing ones.
- **Pressure scenario** (`sjálfsmorðshugleiðingar`, 28 translations): the visible cap + auto-fit floor are what keep this from blowing up. Hardware-test this case explicitly.

## Verification

- `make test` — pure helpers (`split_translations`, `cap_translations`, `fit_headword_size`, `weather_label`, `pos_long`).
- `make validate` + `make compile` — YAML and includes still build.
- Hardware: render all three scenarios against the dev-mode toggle (15 min refresh) and compare to `preview/index.html` side-by-side.

## Steps

1. Add fonts to `fonts.package.yaml`; verify glyph coverage.
2. Extract pure helpers into `einkframe_utils` with doctests.
3. Replace renderer state structs and panel functions; constants `static` in `.cpp`.
4. Update `einkframe.yaml` display lambda to call the new functions.
5. Compile, validate, deploy to hardware. Iterate on Y/X constants.
6. Write `task-summary.md`, note any `ARCHITECTURE.md` updates (new helpers, new panel functions).
