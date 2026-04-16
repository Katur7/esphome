# 02 — Battery Calibration Summary

## Actions Taken

### Phase 1 — Data collection setup
- Temporarily exposed `battery_adc` to HA (removed `internal`, added `name`)
- Added hourly battery read timer at :30

### Phase 2 — Data collection
- Charged fully, then drained to shutdown
- Observed range: 4.14V (full, settled) → 3.09V (dead)
- Total runtime: ~2.5 days
- Data saved in charging.csv and draining.csv

### Phase 3 — New lookup table
- Replaced piecewise lookup with values derived from actual discharge curve
- Updated voltage clamps: 3.09V–4.14V (was 3.30V–4.20V)
- Removed stale calibration factor TODO

### Cleanup
- Reverted `battery_adc` to internal (no longer needed in HA)
- Removed hourly battery read timer

## Lessons Learned

- Old lookup table was way off — at 3.58V it reported 5% when real data shows ~10%
- The ADC multiply factor (x2) is accurate enough, no calibration needed

## Architecture Update

- Updated ARCHITECTURE.md with TRMNL kit link and battery specs.
