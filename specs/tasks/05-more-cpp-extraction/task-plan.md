# Task 05 — More C++ Logic Extraction

## Goal

Extend the testable C++ surface by extracting two more pieces of logic out of YAML/ESPHome-coupled code: the weather-state icon lookup, and the text-wrapping algorithm. Retire the "can't test text_utils" caveat from task 04.

## Scope

**In:**
- Extract weather-state → MDI icon lookup into `einkframe_utils`
- Extract the generic `icon_map` (cloud-percent, umbrella, sun, battery) into `einkframe_utils`
- Refactor `wrap_text` to decouple the wrapping algorithm from `esphome::font::Font` via an injected measurer
- Tests for both
- Wire refactored code back into display lambda and confirm firmware still renders correctly

**Out:**
- `get_text_width` itself stays Font-coupled (it *is* the Font call)
- Display layout/coordinate math
- `wait_for_all_sensors` sensor-readiness gate
- `next_wake_hour` / `atof` wrappers — too trivial to pay for

## Decisions

- **Icon lookup placement:** new functions in `einkframe_utils`, not a new module. Still small enough to share a home with sleep/battery helpers. Revisit if it grows.
- **Two separate functions, not one generic lookup.** `weather_icon` maps HA weather states (external vocabulary); `ui_icon` maps our internal UI keys (`mdi-cloud-percent`, etc.). Different domains, different evolution — keep them apart even though the mechanism is identical.
- **Unknown key:** both return empty string (current behavior — `std::map::operator[]` on a missing key yields default-constructed string). Make that explicit and tested.
- **wrap_text refactor shape:** pure algorithm lives in `einkframe_utils` and takes a measurer callable (`std::function<int(const std::string&)>`). A thin `wrap_text(Font*, text, max_width)` stays in `text_utils` and forwards to the pure function with a Font-backed measurer. Existing YAML call sites unchanged.
- **get_text_width stays where it is.** It's the Font adapter. No test.

## Structure

```
config/packages/einkframe/
  einkframe_utils/
    einkframe_utils.h         (+ weather_icon, + ui_icon, + wrap_text_pure)
    einkframe_utils.cpp
  text_utils/
    text_utils.{h,cpp}        (wrap_text becomes thin forwarder)
test/
  test_einkframe_utils.cpp    (+ icon tests, + wrap_text tests)
```

## Functions to Extract

- `std::string weather_icon(const std::string& state)` — HA weather state → MDI codepoint string. Unknown → `""`.
- `std::string ui_icon(const std::string& key)` — internal UI key (`mdi-cloud-percent`, `mdi-umbrella`, `mdi-sun-wireless-outline`, `mdi-battery`) → MDI codepoint string. Unknown → `""`.
- `std::vector<std::string> wrap_text_pure(const std::string& text, int max_width, <measurer>)` — algorithmic wrapping, no Font dependency. Measurer maps string → width.

## Test Coverage

`weather_icon`:
- Known states (sunny, rainy, partlycloudy, snowy-rainy — mix of simple and hyphenated)
- Unknown state → empty string
- Empty string input → empty string

`ui_icon`:
- All four known keys
- Unknown key → empty string

`wrap_text_pure` (with a fake measurer, e.g. 1 char = 1 unit):
- Single word, fits → one line
- Multiple words, all fit → one line
- Wraps at word boundary when line exceeds width
- Single word longer than max_width → emitted on its own line (current behavior — document it)
- Empty input → empty vector
- Trailing/leading whitespace handling (current `istringstream >> word` drops it — lock that in)

## Implementation Order

1. `weather_icon` + `ui_icon`: tests → implement → replace both inline maps in display lambda.
2. `wrap_text_pure`: test with fake measurer → implement → have existing `wrap_text(Font*, …)` forward to it.
3. Validate: `make test`, `make validate`, `make compile`, upload, confirm display still renders icons + wrapped text correctly.

## Open Questions

None at plan time.
