# Task 04 — C++ Tests & Logic Extraction

## Goal

Set up native C++ unit testing infrastructure and extract pure logic out of YAML lambdas into a testable C++ module, to prepare for moving more logic to C++.

## Scope

**In:**
- Add doctest-based native test runner
- `make test` target (g++, C++17, compile + run in one step)
- Extract sleep time math from `enter_sleep` lambda
- Extract battery voltage→percent lookup from `battery_percent` lambda
- Wire extracted functions back into YAML
- Verify firmware still compiles and runs on device

**Out:**
- Refactoring `text_utils` (tightly coupled to ESPHome `Font`)
- Testing display rendering lambda (coupled to ESPHome `Display`)
- Docker-based test runs
- CI pipeline

## Structure

```
config/packages/einkframe/
  text_utils/                    (unchanged)
  einkframe_utils/
    einkframe_utils.h
    einkframe_utils.cpp
test/
  doctest.h                      (vendored single header)
  test_einkframe_utils.cpp
Makefile                         (add `test` target)
```

Single shared module for extracted pure logic; split later if it grows.

## Functions to Extract

- `int minutes_to_next_slot(int hour, int minute)` — minutes from given time to next wake slot in `{7, 11, 15, 19}`, wrapping to next day's 07:00 after 19:00.
- `float voltage_to_battery_percent(float voltage)` — maps voltage to percent via measured discharge curve; clamps to [3.09, 4.14]; returns NaN for `voltage <= 0`.

Both are pure (no ESPHome dependencies). The YAML lambdas become thin wrappers that call these and pipe results into ESPHome APIs.

## Test Coverage

`minutes_to_next_slot`:
- Normal gaps between slots (e.g. 08:30 → 150)
- Exactly on a slot (07:00 → 240, not 0)
- Edge minute before a slot (06:59 → 1)
- Midnight (00:00 → 420)
- After last slot (19:01 → 719, wraps to 07:00 next day)

`voltage_to_battery_percent`:
- Exact lookup values (4.13 → 100, 3.49 → 5)
- Between-tier values (4.12 → 95)
- Clamp high (4.20 → 100)
- Clamp low (2.50 → 0)
- Invalid (0, negative → NaN)
- Boundaries (3.09 → 0, 4.14 → 100)

## Implementation Order

1. Scaffold: vendor `doctest.h`, add empty `einkframe_utils.{h,cpp}`, add `make test` target, verify empty test run passes.
2. Extract `minutes_to_next_slot`: write tests first, implement, update YAML lambda, run tests.
3. Extract `voltage_to_battery_percent`: same pattern.
4. Wire YAML: add `einkframe_utils` to `esphome.includes`, replace lambda bodies.
5. Verify: `make validate`, `make compile`, upload to device, confirm sleep schedule and battery % still work.

## Open Questions

None at plan time.
