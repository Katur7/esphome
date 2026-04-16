# 03 — Deep Sleep

## Goal

Frame deep sleeps between refreshes. HA switch to disable sleep for development/OTA.

## Sleep Schedule

- Schedule: 07, 11, 15, 19 (4h intervals, no overnight refreshes)
- On each wake: connect WiFi/HA, refresh display, check sleep switch, sleep or stay awake
- Sleep duration calculated dynamically to hit next slot (12h overnight gap after 19)
- Reset button = manual wake

## Dev Mode

- HA-exposed switch, default off (sleep enabled)
- When on: stays awake, refreshes every 15 min, OTA/logs work normally
- Survives reboots

## Changes

1. Add `deep_sleep` component with manual enter (no auto `run_duration`)
2. Add dev mode switch using `deep_sleep.prevent`/`deep_sleep.allow`
3. After refresh cycle: calculate time to next slot, enter deep sleep (unless dev mode)
4. Replace current time-based schedule with 15-min dev mode refresh

## Out of Scope

- GPIO wakeup (reset key sufficient)
- Battery read changes (separate task)
