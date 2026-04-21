# 03 — Deep Sleep Summary

## Actions Taken

- Added `deep_sleep` component with manual enter (no auto run_duration)
- Added `enter_sleep` script — calculates minutes to next slot in [07, 11, 15, 19], wraps to 07:00 after 19:00
- Modified boot sequence: refresh → enter_sleep
- Replaced fixed-hour schedule with 15-min dev mode refresh
- Updated CLAUDE.md commands to use make targets

## Dev Mode Redesign

Initial implementation used an ESPHome template switch. Problem: when the frame is asleep, HA shows the entity as unavailable and refuses to toggle it — classic chicken-and-egg.

Flipped ownership: source of truth is now an `input_boolean.einkframe_dev_mode` in HA. ESP subscribes via `binary_sensor` (homeassistant platform) and calls `deep_sleep.prevent`/`allow` on state change. HA entity is always available, so toggling works while the frame sleeps. Reset button wakes the frame, which then reads the new state on boot.

Requires `input_boolean.einkframe_dev_mode` to exist in HA (Helpers or configuration.yaml).

## Lessons

- For any ESP-side switch that gates deep sleep behavior, HA must own the state. An ESPHome-owned entity becomes unreachable during sleep.
- `deep_sleep.prevent`/`allow` driven by `on_state` handler of the HA subscription is the clean pattern.

## Architecture Update

- ARCHITECTURE.md updated with deep sleep schedule and HA-owned dev mode mechanism.
