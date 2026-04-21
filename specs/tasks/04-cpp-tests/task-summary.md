# Task 04 Summary — C++ Tests & Logic Extraction

## Actions

- Vendored `doctest.h` v2.4.11 as single header in `test/`.
- Added `make test` target (g++, C++17, `-Wall -Wextra`). Single target compiles + runs in one step.
- Created new module `einkframe_utils` for pure, ESPHome-free logic. Added to `esphome.includes`.
- Extracted `minutes_to_next_slot(hour, minute)` from `enter_sleep` lambda. YAML now calls the function.
- Extracted `voltage_to_battery_percent(voltage)` from `battery_percent` lambda (sensors.package.yaml). YAML is now a one-liner.
- Wrote 11 test cases / 26 assertions covering: normal gaps, exact-slot skipping, midnight wrap, post-last-slot wrap, exact & between-tier lookup values, high/low clamps, NaN for invalid voltage.
- Added `test/test_runner` to `.gitignore`.
- Updated `specs/ARCHITECTURE.md`: file tree, new "Custom C++" subsection covering both modules, new "Testing" section.

## Lessons Learned

- doctest is a good fit: zero build system, one file, ~7k lines, done. `#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` + `#include "doctest.h"` is the whole setup.
- `esphome.includes` is already project-wide — functions defined in one included module are visible from lambdas anywhere in the YAML tree, including nested packages (confirmed by the battery lambda in `sensors.package.yaml` calling into `einkframe_utils`).
- TDD loop was fast: write tests → red → implement → green, for each function. Red phase caught the missing declaration on the first attempt — a real sanity check, not theater.
- Keeping `text_utils` out of scope was correct: `get_text_width`/`wrap_text` are tightly coupled to `esphome::font::Font` and can't be tested natively without mocking or a significant refactor.

## Architecture Impact

Updated `specs/ARCHITECTURE.md` — added `einkframe_utils/` and `test/` to the file tree, split "Custom C++" to describe both modules, and added a "Testing" section.
