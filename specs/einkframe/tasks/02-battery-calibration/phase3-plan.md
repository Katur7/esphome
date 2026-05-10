# 02 Phase 3 — Battery Lookup Table

## Data

From charging.csv and draining.csv:
- **Full (settled)**: ~4.13V
- **Dead (last reading)**: ~3.09V
- **Total runtime**: ~2.5 days

## Plan

Replace the piecewise lookup table in `sensors.package.yaml` with one based on actual discharge data.

Approach: sample the discharge curve at even percentage intervals. 83 data points over ~67h total runtime. Map elapsed time linearly to 100%→0%, pick the voltage at each 5% step.

| % | Voltage |
|---|---------|
| 100 | 4.13 |
| 95 | 4.11 |
| 90 | 4.08 |
| 85 | 4.05 |
| 80 | 4.02 |
| 75 | 4.00 |
| 70 | 3.98 |
| 65 | 3.96 |
| 60 | 3.93 |
| 55 | 3.90 |
| 50 | 3.87 |
| 45 | 3.84 |
| 40 | 3.80 |
| 35 | 3.77 |
| 30 | 3.72 |
| 25 | 3.68 |
| 20 | 3.64 |
| 15 | 3.61 |
| 10 | 3.56 |
| 5 | 3.49 |
| 0 | 3.09 |

Also update voltage clamps: min 3.09V, max 4.14V.
