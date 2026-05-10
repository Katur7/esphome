# 02 — Battery Calibration

## Problem

Battery percentage is unreliable. At 3.58V the lookup table says 5%, but for a standard Li-ion that voltage is more like 30-40%. Either the ADC multiply factor is wrong, the lookup table is wrong, or both.

## Hardware

- TRMNL 7.5" DIY kit, 2000mAh Li-ion battery (3.0–4.2V nominal)
- PCB has voltage divider feeding GPIO1, ESPHome reads with `attenuation: 12db`, `multiply: 2`
- Existing TODO in code: "maybe needs calibration factor 0.968"

## Plan

### Phase 1: Expose raw voltage to HA

- Make `battery_adc` sensor non-internal so it reports to HA
- Keep `battery_percent` as-is for now (will fix after data collection)

### Phase 2: Collect data (manual)

- Charge battery fully, let it drain over days
- HA history will show the actual voltage range
- Optional: measure battery terminals with multimeter at a known charge state, compare to ADC reading to verify the multiply factor

### Phase 3: Fix (separate task)

- Use collected min/max to build correct lookup table or linear mapping
- Fix multiply factor if multimeter shows discrepancy

## Changes (Phase 1 only)

- `sensors.package.yaml`: remove `internal: true` from `battery_adc`
- Give it a proper `name` so it shows up in HA
