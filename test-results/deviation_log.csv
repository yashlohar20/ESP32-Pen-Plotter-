# Test Results — ESP32 Pen Plotter

## Summary

| Metric | Initial | Final | Improvement |
|--------|---------|-------|-------------|
| Max X Deviation | 1.8 mm | 0.8 mm | ~56% |
| Max Y Deviation | 2.1 mm | 0.8 mm | ~62% |
| Overall Precision | >2 mm | <1 mm | >25% |
| Test Cycles | — | 30+ | — |
| Extended Runtime | Unstable | Stable | Drift-free |

## Test Methodology

- Each test run executed the same programmed pattern (Square or Nikolaus House)
- Deviation measured manually using digital calipers at four corner points
- PWM speed values adjusted between runs based on measured deviation data
- Final values validated over 30+ consecutive cycles without recalibration
- Extended runtime test conducted over 30 minutes to check for thermal drift

## Key Findings

1. **Asymmetric X-axis speed** — right vs. left motion required different PWM values
   (220 right, 200 left) due to mechanical friction asymmetry in the X rail
2. **Y-axis more consistent** — Y motor showed lower variance across runs
3. **Diagonal patterns** (Nikolaus House) performed within same tolerance as straight moves
4. **No drift detected** over extended runtime — thermal effects on motor drivers negligible
   at operating speeds

## Final Tuned Parameters

```cpp
const int pwmSpeedXRight = 220;   // Asymmetric — right direction
const int pwmSpeedXLeft  = 200;   // Asymmetric — left direction
const int pwmSpeedYUp    = 220;
const int pwmSpeedYDown  = 220;
const int pwmSlowSpeedX  = 210;   // Used during homing
const int pwmSlowSpeedY  = 210;   // Used during homing
```
