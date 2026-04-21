# Task 07 — Renderer Utils Refactor

## Goal

Reduce duplication and clarify intent in [config/packages/einkframe/renderer_utils/renderer_utils.cpp](../../../config/packages/einkframe/renderer_utils/renderer_utils.cpp). Output must remain pixel-identical.

## Scope

**In:**
- Helpers for the "wrap-then-draw-lines" pattern (5 repetitions today)
- Helper for the info-column row pattern (4 rows today)
- Named constants for layout numbers and the `"  - "` prefix
- Remove the `"unknown"` string sentinel from the renderer by changing the example slots to a filtered list

**Out:**
- Adding unit tests for `renderer_utils` (still coupled to ESPHome types — same reason it sat outside the doctest harness in task 06)
- Coordinate/layout changes — visual output must not move
- A `Layout` struct or supporting multiple layouts

## Decisions

- **Two wrapped-line helpers, not one-with-optional-prefix.** `draw_wrapped_block` (plain) and `draw_prefixed_wrapped_block` ("  - " prefix at the anchor, continuations indented by the prefix width). Explicit beats a magic empty-string prefix. Both take a `y` reference and advance it.
- **One `draw_info_row` helper for the left column.** Signature: `(it, y, mdi_font, icon, value_font, value_text)`. The rain row's trailing umbrella stays as a separate inline call at its site — a one-off, not worth a "trailing icon" parameter.
- **Caller formats the value text.** `draw_info_row` takes a plain `const std::string&`, not a format string + args. Keeps `snprintf`/`atof` conversions visible in the panel bodies instead of hiding them in a variadic helper.
- **Constants at file scope as `constexpr`.** Named individually (`ROW_WEATHER_Y = 160`, `INFO_COL_ICON_X = 130`, `WRAP_WIDTH = 400`, `LINE_HEIGHT = 25`, etc.). Four row y-coords by name, not an indexed array — there are only four and names document intent.
- **Example slots become a `std::vector<ExamplePair>`.** `struct ExamplePair { std::string is; std::string se; }`. The YAML lambda builds the vector and filters out pairs where `is == "unknown"` before handing it off. Renderer loops without knowing about sensor conventions.
- **Helpers are `static` functions in the `.cpp`.** Implementation detail, no need to export.

## Structure

```
config/packages/einkframe/renderer_utils/
  renderer_utils.h     (+ ExamplePair; WordOfDayState.examples changes type)
  renderer_utils.cpp   (+ static helpers, + constexpr constants)
config/einkframe.yaml  (lambda: build + filter examples vector)
```

## Implementation Order

1. Introduce `constexpr` constants, mechanical rename of literals. Compile.
2. Extract `draw_wrapped_block` / `draw_prefixed_wrapped_block`, replace the five call sites. Compile.
3. Extract `draw_info_row`, replace the four rows. Compile.
4. Change `WordOfDayState.examples` to `std::vector<ExamplePair>`. Update the lambda to build and filter. Remove the `"unknown"` branch in the renderer. Compile.
5. `make validate` + `make compile` + upload + visual diff against the current render.

## Open Questions

None at plan time.
