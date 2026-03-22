# ESP32 2-Axis Pen Plotter — Embedded Motion Control System

> A compact CNC-style pen plotter built on ESP32 with PWM motor control, 6-step homing,
> hardware emergency stop, and a browser-based WiFi control interface.
> Achieved **<1 mm positioning deviation** over 30+ repeated test cycles through
> systematic firmware tuning and structured validation.

---

## Project Overview

This project was developed as part of my Mechatronics & Robotics Master's programme at
Hochschule Schmalkalden. The goal was to design and validate a low-cost, reliable 2-axis
motion platform — demonstrating how embedded firmware, motor control, and structured
test engineering can be combined to achieve repeatable, precise mechanical motion.

The system draws programmed shapes (squares, diagonals, the Nikolaus House figure) and
is controlled either via Serial terminal or a browser-based web interface served directly
from the ESP32 over WiFi — no router required.

---

## Key Results

| Metric | Value |
|--------|-------|
| Final positioning deviation | **< 1 mm** (both axes) |
| Precision improvement vs. baseline | **> 25%** through iterative tuning |
| Validation test cycles | **30+** repeated runs |
| Extended runtime stability | **No drift** detected over 30-minute session |
| Control interface | Serial terminal + Browser (WiFi soft AP) |

---

## System Architecture

```
┌─────────────────────────────────────────────────┐
│                  ESP32 DevKit                    │
│                                                  │
│  PWM Ch 0,1 ──► DRV8833 ──► DC Motor (X axis)  │
│  PWM Ch 2,3 ──► DRV8833 ──► DC Motor (Y axis)  │
│                                                  │
│  GPIO 32,33 ◄── X Limit Switches (left/right)  │
│  GPIO  4, 5 ◄── Y Limit Switches (top/bottom)  │
│  GPIO    13 ◄── Emergency Stop Button           │
│  GPIO     2 ──► Driver SLEEP + Status LED       │
│                                                  │
│  WiFi Soft AP ──► Browser Control Interface     │
│  UART Serial  ──► Terminal Command Interface    │
└─────────────────────────────────────────────────┘
```

---

## Firmware Features

### Motion Control
- **PWM-based DC motor control** via ESP32 LEDC peripheral (10 kHz, 8-bit resolution)
- **Asymmetric speed tuning** — different PWM values for left vs. right X-axis motion
  to compensate for mechanical friction asymmetry in the rail
- **Simultaneous XY motion** for diagonal drawing via `moveXYTimed()`
- **Motor settle delays** after each move to eliminate overshoot

### Homing Routine (6-step)
1. Move X to right limit switch
2. Move X to left limit switch (measures full travel)
3. Move Y to bottom limit switch
4. Move Y to top limit switch (measures full travel)
5. Move to bottom-left corner (origin)
6. Offset right from X-left by 1 second (clear switch for consistent start)

### Safety
- **Hardware emergency stop** on GPIO 13 — checked inside every motion loop
- **Immediate motor cutoff** + driver sleep on emergency trigger
- **ESP32 hard restart** after emergency to restore clean system state
- **Limit switch debouncing** (50 ms) to prevent false triggers
- **Maximum move time timeout** (15 s) to prevent runaway motion

### Web Interface
- ESP32 acts as a **WiFi soft access point** (`ESP32-PLOTTER` / `12345678`)
- Browser connects directly — no router required
- **Real-time status polling** every 1 second via `/status` endpoint
- **Canvas animation** previews drawing before execution
- Commands: Home, Draw Square, Nikolaus House, Center, Emergency Stop, Reset

---

## Repo Structure

```
esp32-pen-plotter/
├── firmware/
│   ├── src/
│   │   └── main.cpp          # Main firmware — motion control, homing, commands
│   ├── include/
│   │   └── websurface.h      # WiFi soft AP + web server + browser UI
│   └── platformio.ini        # PlatformIO build configuration
├── hardware/
│   └── BOM.md                # Bill of materials + pin assignment table
├── test-results/
│   ├── deviation_log.csv     # Raw test data — 25 runs, PWM vs. deviation
│   └── README.md             # Test methodology, findings, tuned parameters
└── README.md                 # This file
```

---

## How to Build & Flash

### Requirements
- [PlatformIO](https://platformio.org/) (VS Code extension recommended)
- ESP32 DevKit v1
- USB cable for flashing

### Steps
```bash
# Clone the repo
git clone https://github.com/[your-handle]/esp32-pen-plotter.git
cd esp32-pen-plotter/firmware

# Build and flash
pio run --target upload

# Open serial monitor
pio device monitor --baud 115200
```

### First Run
1. Flash firmware and open Serial Monitor at 115200 baud
2. Type `home` and press Enter — the system will execute the 6-step homing routine
3. Once homed, type `square` or `nikolaus` to draw
4. Or connect to WiFi `ESP32-PLOTTER` (password: `12345678`) and open `192.168.4.1` in browser

---

## Serial Commands

| Command | Description |
|---------|-------------|
| `home` | Execute full 6-step homing routine |
| `square` | Draw a square (requires homing first) |
| `nikolaus` | Draw the Nikolaus House figure |
| `center` | Move to calculated center position |
| `status` | Print current system status |
| `stop` | Stop all motors and disable driver |
| `reset` | Hard restart ESP32 |
| `help` | List available commands |

---

## Test Methodology

Positioning accuracy was measured by running each programmed pattern, then measuring
the deviation at key points using digital calipers. PWM speed values were adjusted
between runs based on measured results — not intuition.

Key finding: **X-axis required asymmetric PWM** (220 right, 200 left) due to friction
asymmetry in the linear rail. This was identified through systematic testing, not visible
during build.

Full test data: [`test-results/deviation_log.csv`](test-results/deviation_log.csv)

---

## What I Learned

- **Mechanical imperfections require firmware compensation** — the friction asymmetry
  discovery showed that reliable motion requires characterising the hardware, not just
  writing theoretically correct code
- **Structured test campaigns matter** — random tuning would have taken much longer
  to reach <1 mm; a systematic test matrix found the optimum in ~25 runs
- **Safety-first firmware design** — embedding emergency stop checks inside every
  motion loop (not just at the top level) is essential for any physical system
- **Web interfaces on embedded hardware** are surprisingly achievable with ESP32 —
  the soft AP approach works without any external infrastructure

---

## Author

**Yash Lohar**
M.Eng. Mechatronics and Robotics — Hochschule Schmalkalden, Germany
[linkedin.com/in/yash-lohar](https://linkedin.com/in/yash-lohar)

---

## License

MIT License — free to use, modify, and build upon with attribution.
