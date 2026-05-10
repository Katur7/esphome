# Task 06 — Display Panel Extraction

## Goal

Break up the ~120-line display lambda in [config/einkframe.yaml](../../../config/einkframe.yaml) into panel draw functions in a new `renderer_utils` module. Lambda becomes thin orchestration: read state, call panels.

## Scope

**In:**
- New module `config/packages/einkframe/renderer_utils/` with `.h` / `.cpp`
- Three draw functions: `draw_title`, `draw_info_column`, `draw_word_of_day`
- Per-panel state structs (`TitleState`, `InfoColumnState`, `WordOfDayState`)
- Wire display lambda to call the new functions
- Verify firmware renders identically

**Out:**
- Layout/coordinate changes — output must be pixel-identical
- Additional testable logic extraction (covered by task 05)

## Decisions

- **Module name:** `renderer_utils`, matches the existing `*_utils` schema.
- **Title stays its own function** even though it's one `printf` — symmetry and keeps the lambda fully declarative.
- **Per-panel structs, not a shared `FrameState`.** Each panel's inputs documented by its own struct; no cross-panel coupling via a shared blob.
- **Fonts passed positionally, not via struct.** They don't change per render and adding a struct for them is noise.
- **Icon lookups via `weather_icon` / `ui_icon`** (from task 05) — panels call these, not inline maps.
- **Text wrapping via `wrap_text`** (existing Font-backed forwarder) — panels don't touch the pure algorithm.
- **Not added to doctest harness.** Module `#include`s `esphome/components/display/display_buffer.h` and `font/font.h`, same reason `text_utils` sits outside the harness. Pure math stays in `einkframe_utils`.

## Structure

```
config/packages/einkframe/
  renderer_utils/
    renderer_utils.h     (structs + draw function declarations)
    renderer_utils.cpp   (implementations)
```

## Panel Responsibilities

- **`draw_title`** — the date, centered top. Needs: date string, title font.
- **`draw_info_column`** — left column at x=100–140: weather icon + temp, rain icon + %, UV icon + value, battery icon + %. Needs: weather state, temp, rain, UV, battery percent, subheading/normal/mdi fonts.
- **`draw_word_of_day`** — right column at x=300: heading, Icelandic word, wrapped translations, wrapped example pairs (example2 optional). Needs: word, translations, example1 is/se, example2 is/se, subheading/normal/smaller fonts.

## Implementation Order

1. Depends on task 05 landing first (uses `weather_icon` / `ui_icon`).
2. Create `renderer_utils.{h,cpp}` with structs and empty function bodies.
3. Add include to [config/einkframe.yaml](../../../config/einkframe.yaml) `esphome.includes`.
4. Move title code into `draw_title`, replace in lambda, `make compile`, upload, eyeball.
5. Repeat for `draw_info_column`, then `draw_word_of_day`.
6. Final lambda should be: read state into structs, call three draw functions, done.
7. `make validate` + `make compile` + upload + visual diff against current render.

## Open Questions

None at plan time.
