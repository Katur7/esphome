# eInkFrame — Display Design

The chosen layout for the Waveshare 7.5″ v2 e-paper frame: **Final A** — a left-aligned editorial dictionary entry framed by a Swiss-style header rule and a 4-cell info strip.

![Final A — chosen layout](preview.png)
*(Render `eInkFrame Layouts.html` → "Finalists" → Final A. Mockups for all explored directions live in the same file.)*

---

## Display constraints

| Property      | Value                                                |
| ------------- | ---------------------------------------------------- |
| Hardware      | Waveshare 7.50inv2 e-paper                           |
| Resolution    | **800 × 480 px**                                     |
| Color depth   | **1-bit** — pure black or pure white, no greyscale   |
| Refresh model | Deep-sleep between updates; refresh ~every 4 hours   |
| Implication   | Strong contrast, hairline-thick rules, no gradients  |

---

## Layout structure (top → bottom)

```
┌───────────────────────────────────────────────────────────────┐
│ [date — Inter 900, 30px]              [☀ icon] [12° — 42px]  │  70 px  HEADER
├───────────────────────────────────────────────────────────────┤  3 px rule
│                                                               │
│  ORÐ DAGSINS · ÍSLENSKA → SVENSKA                    № 01     │  10 px label
│  ───────────────────────────────────────────────────────────  │  hairline rule
│                                                               │
│  óstöðugur                                            ENTRY   │  Fraunces 900
│  lo. (lýsingarorð) · svenska översättningar                   │  italic, 14
│                                                               │
│  ostadig, ostabil, instabil                                   │  Fraunces 600, 24
│                                                               │
│  hann gekk óstöðugur á fótunum upp að húsinu                  │  Fraunces italic
│  — han gick fram till huset på ostadiga ben                   │  Fraunces, w/ dash
│                                                               │
├───────────────────────────────────────────────────────────────┤  3 px rule
│ REGN  │ UV-INDEX │  VÄDER          │ BATTERI                  │  60 px STRIP
│ ☔ 23%  │ ☀ 4.2   │  ☁ växlande    │ 🔋 78%                   │
└───────────────────────────────────────────────────────────────┘
```

### 1. Header (70 px)
- Full-width strip, divided 1fr / auto by a 3 px vertical rule
- **Left**: Swedish long-form date in Inter 900, 30 px, tight tracking
- **Right**: weather MDI icon (48 px) + temperature in Inter 900, 42 px
- Bottom border: 3 px black rule

### 2. Entry block (full width, ~350 px)
- **Section label** in JetBrains Mono uppercase, with `№ 01` index on the right
- **Hairline rule** (1 px) under the label
- **Word**: Fraunces 900 italic-eligible serif, 60 px, ranged left, tight tracking
- **POS line**: Fraunces 400 italic, 14 px, e.g. `lo. (lýsingarorð) · svenska översättningar`
- **Translations**: Fraunces 600, 24 px — comma-separated Swedish translations
- **Examples**: full-width block with NO border or rule
  - **IS line**: Fraunces 600 italic, 15 px
  - **SE line**: Fraunces 400, 15 px, hanging 12 px black dash in 20 px gutter

### 3. Info strip (60 px)
- Four equal cells separated by 1 px hairlines, framed by a 3 px top rule
- Each cell: tiny uppercase mono label (9 px) + value with MDI icon
- Cells: **Regn** (rain %), **UV-index**, **Väder** (label), **Batteri** (or **Uppdaterad** if battery hidden)

---

## Type system

| Use                        | Family            | Weight    | Size    |
| -------------------------- | ----------------- | --------- | ------- |
| Date, temperature, strip   | Inter             | 900       | 30 / 42 / 26 px |
| Section labels, indices    | JetBrains Mono    | 700       | 9–11 px |
| Word                       | Fraunces          | 900       | 60 px   |
| POS / metadata             | Fraunces italic   | 400       | 14 px   |
| Translations               | Fraunces          | 600       | 24 px   |
| Example IS                 | Fraunces italic   | 600       | 15 px   |
| Example SE                 | Fraunces          | 400       | 15 px   |

The current ESPHome config ships with **Inter Bold** at 32 / 24 / 20 / 16. To match the mock we'll need to add **Fraunces** (3 weights) and **JetBrains Mono** (1 weight) — both available via `gfonts://`.

---

## Iconography

- **Material Design Icons** (MDI), already integrated via `font_mdi_medium`
- Used for: weather state, umbrella (rain), UV, battery
- Size: 48 px in header, 22 px in strip
- No custom SVG drawing — everything comes from the existing MDI glyph set

---

## Why this layout

- **Word of the day is the hero** — it occupies the largest visual area, with the most expressive type (serif italic display)
- **Glanceable secondary info** — date and weather frame the top, environmental info pinned to the bottom strip; both legible from across a room
- **Reads like a printed dictionary spread** — full-width entry + examples, hairline rules for hierarchy, no visual noise
- **1-bit-friendly** — all hierarchy comes from type weight, scale, and rules; nothing relies on grey

---

## Data fields used

| Field             | HA source                                  | Where shown          |
| ----------------- | ------------------------------------------ | -------------------- |
| `current_date`    | `sensor.current_date`                      | Header left          |
| `myWeather`       | `weather.smhi_home`                        | Header icon + strip  |
| `myTemperature`   | `weather.smhi_home` attr. `temperature`    | Header right         |
| `expected_rain`   | `sensor.expected_rain`                     | Strip cell 1         |
| `uv_index`        | `sensor.openuv_current_uv_index`           | Strip cell 2         |
| `battery_percent` | template sensor (from `battery_adc`)       | Strip cell 4         |
| `myIcelandicWord` | `sensor.word_of_the_day` attr. `fletta`    | Entry word           |
| `word_translations` | `sensor.word_of_the_day_translations`    | Entry translations   |
| `word_example1_is/se` | `sensor.word_of_the_day_example_1_*`   | Examples block       |
| `word_example2_is/se` | `sensor.word_of_the_day_example_2_*`   | Examples block       |

---

## Porting plan (renderer_utils.cpp)

The current renderer divides the screen into `draw_title`, `draw_info_column`, and `draw_word_of_day`. The new layout reorganizes into:

1. `draw_header(it, fonts, HeaderState{date, weather, temperature})`
   - Date string at `(32, 35)`, font: title (Inter 30)
   - Weather icon + temp at right, separated by 3 px vertical rule
   - 3 px horizontal rule at `y = 70`

2. `draw_entry(it, fonts, EntryState{label, num, word, pos, translations, examples})`
   - Section label at `(32, 88)`, mono 10 px
   - Hairline rule at `y = 105`
   - Word at `(32, 115)`, serif 900 / 60 px
   - POS at `(32, 180)`, italic 14 px
   - Translations at `(32, 215)`, serif 600 / 24 px
   - Example block starts at `y = 260`, line height 22 px
   - SE indent: 20 px hanging dash drawn as a 12 × 1.5 px filled rect

3. `draw_strip(it, fonts, StripState{rain, uv, weatherLabel, battery})`
   - 3 px rule at `y = 420`
   - Four cells of 200 px each at `y = 423–477`
   - Hairlines between cells at `x = 200, 400, 600`

### Fonts to add to `fonts.package.yaml`

```yaml
- file: "gfonts://Fraunces@900"
  id: serifDisplayFont
  size: 60
- file: "gfonts://Fraunces@600"
  id: serifTextFont
  size: 24
- file: "gfonts://Fraunces@400"
  id: serifSmallFont
  size: 15
- file: "gfonts://Fraunces@400italic"
  id: serifItalicFont
  size: 15
- file: "gfonts://JetBrains+Mono@700"
  id: monoLabelFont
  size: 10
```

(Existing `myFont` / `subheaderFont` etc. stay for the header and strip values.)

### Implementation notes
- Use `wrap_text()` for the IS/SE example lines — long words like `ræktunarskilyrði` may wrap
- For the SE dash glyph, draw a filled rectangle: `it.filled_rectangle(x, y_baseline - 4, 12, 2)`
- Keep dev-mode toggle behavior unchanged
- All Y offsets above are starting points; tune visually on hardware after first port

---

## Open questions
- Add **part-of-speech** to the HA template sensor? (Currently the renderer would default the POS line to `lo./no./so.` — could be hardcoded by word or fetched.)
- **Section index** (`№ 01`) — does this come from anywhere meaningful, or is it decorative? Suggest decorative (always `№ 01`) for now.
- Behavior when a word has only **one** example: layout already handles this correctly (block just shorter).

---

## Files

- `eInkFrame Layouts.html` — design canvas with all explored variants
- `variant-06-finalists.jsx` — Final A component (the chosen direction)
- `styles.css` — section `.v-hybrid` + `.v-hybrid-stacked` + `.v-06fa` are the rules in play
- `data.jsx` — sample data shape mirrors the renderer's `WordOfDayState` / `InfoColumnState`

---

## Section 07 — Orð dagsins harmony

Added after reviewing the `handoff 3` design package for the companion
`ord-dagsins.pippinn.me` web project. The two designs already share DNA
(left-aligned editorial dictionary entry, serif headword, mono labels,
hairline rules), but the web design has crystallised several signature
moves we hadn't pulled through.

Section **07** ports them. Two variants:

- **07a — Faithful port**: small-caps `ORÐ DAGSINS · TISDAG 5 MAJ 2026` folio
  header with weather + temp on the right, three peg labels (`Íslenska`,
  `Svenska`, `Dæmi`), Newsreader serif throughout, dotted translation list,
  hairlines between adjacent examples.
- **07b — Light touch**: keeps the Final-A Swiss-block top + 4-cell strip;
  swaps only the entry body to the editorial vocabulary. Smaller renderer
  change if we want a softer port.

### Signature elements ported

| Element | Web | e-ink port |
|---|---|---|
| Folio header pattern | Tracked Inter small-caps under ink hairline | Same, plus weather glyph + temp on the right (07a only) |
| Peg labels | `▪ ÍSLENSKA` / `▪ SVENSKA` / `▪ DÆMI` | Same; Dæmi peg uses outline square to read as a section header rather than a language tag |
| Headword | Newsreader 500, auto-fit `[36, 92]px` | Newsreader 500, auto-fit `[36, 74]px` (07a) / `[36, 62]px` (07b) — same JS algorithm |
| Word class | Spelled out italic (`lýsingarorð`) | Same; takes the inside-parens of the existing `pos` field |
| Inflection line | Italic, muted, smaller | Italic, smaller. **Muted collapses to weight/scale on 1-bit.** Hidden if no `inflections` field |
| Translations | Newsreader 30/34, `·` separators in oxblood italic | Newsreader 24, `·` separators in black italic; each item `nowrap` |
| Examples block | Vertical oxblood rule + hairlines between examples | Vertical black rule (existing pattern) + new hairlines between examples |

### 1-bit collapses

The web design's two greys/colors disappear on a 1-bit panel:

- **Oxblood `#7a1a22` → black**. Pegs, dot separators, vertical example rule
  all become pure black. They still differentiate from the headword by
  scale, not hue.
- **Muted `#6a6055` → italic + smaller scale**. The word-class and SE
  example lines lose their grey but keep their hierarchy via italic
  treatment.

### Data fields touched

If we adopt 07a/07b, the renderer benefits from two optional fields on the
HA template sensor (mirroring the web project's open questions):

- `wordClass` / `posLong` — spelled-out Icelandic word class (`lýsingarorð`,
  `nafnorð`, `sagnorð`). Falls back to whatever the existing `pos` field
  contains if absent.
- `inflections` — string of dot-separated forms. Hidden entirely if empty.

### Fonts to add

```yaml
- file: "gfonts://Newsreader@500"
  id: serifDisplayFont
  size: 64
- file: "gfonts://Newsreader@400italic"
  id: serifItalicFont
  size: 15
- file: "gfonts://Newsreader@400"
  id: serifTextFont
  size: 24
```

Existing Inter / JetBrains Mono fonts stay for the strip and folio header.

### Files

- `variant-ord.jsx` — 07a + 07b components
- `styles.css` — section `.v-ord` + `.v-ord-a` / `.v-ord-b`
