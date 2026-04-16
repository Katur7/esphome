# 01 — Current Setup Review

## Scope

Review all config YAML and custom C++ for bugs, security issues, and improvements.

## Findings

### Bugs

**1. printf format mismatch — will crash or show garbage (critical)**

`expected_rain` and `uv_index` are `text_sensor` (string), but the display lambda uses float format specifiers with `.c_str()`:

- Line 188: `"%.1f%%"` with `id(expected_rain).state.c_str()` — should be `"%s%%"`
- Line 190: `"%.1f%% %.0f"` with `.c_str()` and `int` — both wrong
- Line 194: `"%.0f"` with `id(uv_index).state.c_str()` — should be `"%s"`

This is undefined behavior. Passing `char*` where printf expects `double`.

**2. Missing sensors in wait_for_all_sensors**

`expected_rain` and `uv_index` are used in the display lambda but not checked in the `wait_for_all_sensors` script. Display could render before these have values.

### Security

**3. Credentials in plain text in einkframe.yaml**

API encryption key, OTA password, and AP fallback password are hardcoded in the main YAML. These should be in `secrets.yaml` (which is gitignored). If this repo ever goes public, those are exposed.

### Minor

**4. Commented-out variable shadows sensor id**

Line 124: `// auto current_date = id(current_date).state;` — harmless but confusing since `current_date` is also a sensor id. Clean up dead code.

**5. Log line format mismatch**

Line 190: `expected_rain_length` is `int` but format is `%.0f` (expects double). Should be `%d`.

## Proposed Actions

- [ ] Fix printf format specifiers for text_sensor values
- [ ] Add `expected_rain` and `uv_index` to wait_for_all_sensors
- [ ] Move credentials to secrets.yaml
- [ ] Clean up commented-out code
