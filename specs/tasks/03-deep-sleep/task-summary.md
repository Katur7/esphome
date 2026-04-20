# 03 — Deep Sleep Summary

## Actions Taken

- Added `deep_sleep` component with manual enter (no auto run_duration)
- Added "Dev Mode" template switch — exposed to HA, default off, survives reboots
- Added `enter_sleep` script — calculates minutes to next slot in [07, 11, 15, 19], wraps to 07:00 after 19:00
- Modified boot sequence: refresh → enter_sleep
- Replaced fixed-hour schedule with 15-min dev mode refresh
- Updated CLAUDE.md commands to use make targets

## Architecture Update

- ARCHITECTURE.md needs update: add deep sleep and dev mode to refresh cycle description
