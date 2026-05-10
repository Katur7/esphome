# Handoff — 07a Orð dagsins (faithful port)

This package is for an implementing agent to build the **07a** layout on the
real eInkFrame device renderer.

> Target: Waveshare 7.5″ v2, **800 × 480**, 1-bit black & white.

---

## What you're building

A single layout, three content scenarios. Same template absorbs all three.

| Scenario | Headword | Translations | Examples |
|---|---|---|---|
| Short    | `regn`                       | 2  | 1 |
| Medium   | `fjallganga`                 | 4  | 2 |
| Pressure | `sjálfsmorðshugleiðingar`    | 28 | 2 (long) |

Open `preview/index.html` in a browser. The three boards render side-by-side
in a pan/zoom canvas — click any one's title to focus it fullscreen.

---

## Files

```
handoff-07a/
├── README.md              ← you are here
├── DESIGN.md              ← design rationale (see "Section 07" at the bottom)
└── preview/
    ├── index.html         ← the reference rendering (open this)
    ├── variant-ord.jsx    ← React reference impl for 07a (and 07b)
    ├── variant-swiss.jsx  ← provides weatherLabel() used by 07a's strip
    ├── data.jsx           ← SAMPLE_DATA shape + Icon helper + MDI codepoints
    ├── design-canvas.jsx  ← canvas chrome (not part of the layout itself)
    └── styles.css         ← all CSS; the .v-ord / .ord-* block is what matters
```

---

## What to copy

The 07a layout is fully described by three things:

1. **Component**: `preview/variant-ord.jsx` → `VariantOrdA` (top half of file).
   Ignore `VariantOrdB` — not in scope for this handoff.
2. **Styles**: `preview/styles.css` → the `.v-ord` / `.v-ord-a` / `.ord-*`
   selectors. Other variants' CSS is in the same file but not needed.
3. **Data shape**: `preview/data.jsx` → `SAMPLE_DATA` shows every field the
   layout consumes.

Helpers the component references as globals:

- `Icon` + `MDI` — from `data.jsx`. The renderer uses the
  [Material Design Icons](https://pictogrammers.com/library/mdi/) font;
  codepoints listed in `MDI`.
- `weatherLabel(state)` — from `variant-swiss.jsx`, bottom of file. Maps
  Home Assistant weather states to Swedish words for the strip cell.

---

## Layout anatomy (top to bottom)

```
800 × 480 frame
┌─────────────────────────────────────────────────────────────────┐  y = 0
│  ORÐ DAGSINS · TISDAG 5 MAJ 2026               [⛅]  12°        │  folio header
│  ───────────────────────────────────────────────────────────    │  hairline
│                                                                 │
│  ▪ ÍSLENSKA                       │  ▫ DÆMI                     │
│                                   │                             │
│  fjallganga                       │  við fórum í fjallgöngu …   │  hero
│                                   │  vi gick på en bergs… (it)  │
│  nafnorð                          │  ─────────────────────────  │
│                                   │  fjallgangan tók lengri …   │
│  ▪ SVENSKA                        │  bergsbestigningen tog (it) │
│  bergsbestigning · fjäll-         │                             │
│  vandring · bergvandring …        │                             │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤  hairline
│  REGN   23%   UV-INDEX  4.2   VÄDER  ⛅ klart   BATTERI  77%   │  strip
└─────────────────────────────────────────────────────────────────┘  y = 480
```

### Key constraints (do not soften)

- **Headword auto-fit**. Binary search font-size between `36 px` and `74 px`
  so the word fits on one line if it can. If it cannot fit at 36 px,
  fall back to wrapping at character boundaries — but only at the floor.
  See `useFitHeadword` in `variant-ord.jsx`.
- **Translation cap = 15**. Render the first 15 dot-separated translations,
  then append a tracked `+N fler` tag (Inter 700, 13 px). Cap exists because
  the corpus max is 28 items and the column must not push into the strip.
- **Translation wrapping**. Each translation phrase wraps only at the spaces
  between its own words. A single token must never break mid-character;
  if it doesn't fit, the whole word drops to the next line.
  (CSS: `overflow-wrap: normal; word-break: normal;`)
- **Examples**. List, hairline rule between items, IS in serif, SE in italic
  serif at smaller size. No bullets, no numbering.
- **Strip values**. Inter 900 / 22 px across all four cells so the väder
  word meets the rain / UV / battery numbers in the middle.

### 1-bit collapses

The web project uses oxblood (`#7a1a22`) and a muted grey. On the e-ink
panel those don't exist. The port collapses them like so:

| Web | e-ink |
|---|---|
| Oxblood accent (pegs, separator dot, vertical rule) | Pure black; differentiation by **scale**, not hue |
| Muted grey (word class, SE example line) | Italic + smaller size |

---

## Data contract

```js
{
  // Header
  dateLong: "Tisdag 5 maj 2026",
  weather: "clear-night",     // any HA weather state in MDI keys
  temperature: "12",

  // Entry body
  word: "fjallganga",
  pos: "no. (nafnorð)",       // 07a shows only the part in parens, italic
  inflections: "…",           // optional; rendered italic, smaller. omit if absent
  translations: "a, b, c, …", // comma-separated (or `·`-separated)
  examples: [
    { is: "icelandic sentence", se: "swedish translation" },
    // 0–2 items practical max for the column
  ],

  // Strip
  expectedRain: "23",
  uvIndex: "4.2",
  batteryPercent: 77,
}
```

---

## Fonts

- **Inter** (300, 500, 700, 800, 900) — folio header, peg labels, strip
- **Newsreader** (400, 500, 600, italics) — headword, translations, examples

Both are Google Fonts. The web reference loads them via `styles.css`
`@import`. The renderer should embed the same families (or close metric
matches) when generating the PNG for the panel.

---

## Acceptance: render all three preview scenarios

If your output matches `preview/index.html` for each of:

- `regn` (short)
- `fjallganga` (medium)
- `sjálfsmorðshugleiðingar` (pressure)

…you are done. Sample data for each scenario is hard-coded in
`preview/index.html` (`SHORT`, `MEDIUM`, `PRESSURE` constants).
