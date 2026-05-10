# Task 05 Summary — More C++ Logic Extraction

## Actions

- Added `weather_icon(state)` and `ui_icon(key)` to `einkframe_utils`. Two separate lookups: one for HA weather vocabulary, one for our internal UI keys.
- Replaced both inline `std::map` literals in the display lambda with the new function calls.
- Added `wrap_text_pure(text, max_width, measurer)` to `einkframe_utils` — the font-agnostic wrapping algorithm, with an injected `TextMeasurer` callable.
- Refactored `text_utils::wrap_text` to be a thin adapter: it builds a Font-backed lambda and forwards to `wrap_text_pure`. All existing YAML call sites unchanged.
- Wrote 13 new test cases (38 new assertions): icon lookups (known, unknown, empty), wrap_text_pure (fits, multi-wrap, empty, long-word-alone, whitespace).
- Total test suite: 24 cases / 64 assertions, all passing.
- Updated `specs/ARCHITECTURE.md` — Custom C++ section now reflects the split (text_utils = Font adapter; einkframe_utils = algorithm + lookups) and testing section updated.

## Lessons Learned

- `esphome.includes` directories are added to the compiler include path as *their own roots*, not as siblings of each other. `#include "einkframe_utils.h"` from inside `text_utils.cpp` fails; a relative path (`../einkframe_utils/einkframe_utils.h`) works. Caught only at `make compile` — `make validate` doesn't link. Documented in ARCHITECTURE.md so this doesn't trip the next cross-module include.
- Injecting a `std::function<int(const std::string&)>` measurer is all it took to untangle `wrap_text` from `esphome::font::Font`. Thin Font-facing adapter stays in `text_utils`; pure algorithm is fully testable with a `length()` measurer.
- While porting the algorithm, fixed a latent bug in the original: a single word longer than max_width used to produce `["", "long_word"]` (empty leading line). Added an `if (!current_line.empty())` guard before flushing. Behavior in practice unchanged — the 400px word-of-the-day column doesn't have long-enough single words to hit it — but the tests now lock in the cleaner output.
- TDD loop held up again: tests-first on each function surfaced the `""` vs default-constructed return contract immediately.

## Architecture Impact

Updated `specs/ARCHITECTURE.md`:
- "Custom C++" section now describes `text_utils` as a thin Font adapter (algorithm lives in einkframe_utils) and lists the new `einkframe_utils` exports.
- Added a note about the cross-module include path gotcha.
- "Testing" section updated to reflect that the wrapping algorithm is now covered; only the Font adapter and display lambda remain ESPHome-bound.
