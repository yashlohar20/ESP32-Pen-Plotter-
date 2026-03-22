# Bill of Materials — ESP32 Pen Plotter

| # | Component | Part / Model | Qty | Key Specs | Role |
|---|-----------|-------------|-----|-----------|------|
| 1 | Microcontroller | ESP32 DevKit v1 | 1 | 240 MHz dual-core, WiFi/BT, 38 pins | Main controller & web server |
| 2 | Motor Driver | DRV8833 dual H-bridge | 1 | 1.5A per channel, 2.7–10.8V | X and Y axis motor control |
| 3 | DC Motor (X axis) | N20 gear motor | 1 | 6V, ~100 RPM | X-axis linear motion |
| 4 | DC Motor (Y axis) | N20 gear motor | 1 | 6V, ~100 RPM | Y-axis linear motion |
| 5 | Limit Switch (X left) | Mechanical microswitch | 1 | NO/NC, 5V logic | X-axis left boundary |
| 6 | Limit Switch (X right) | Mechanical microswitch | 1 | NO/NC, 5V logic | X-axis right boundary |
| 7 | Limit Switch (Y bottom) | Mechanical microswitch | 1 | NO/NC, 5V logic | Y-axis bottom boundary |
| 8 | Limit Switch (Y top) | Mechanical microswitch | 1 | NO/NC, 5V logic | Y-axis top boundary |
| 9 | Emergency Stop | Latching pushbutton | 1 | NO, panel mount | Hardware safety cutoff |
| 10 | Power Supply | 5V USB / bench supply | 1 | 5V, min 2A | System power |
| 11 | Pen holder | 3D printed / custom | 1 | — | Mounts pen to carriage |
| 12 | Frame / Rails | Custom mechanical | 1 | — | 2-axis motion platform |
| 13 | Connecting wires | Dupont jumper wires | — | 22 AWG | Signal and power connections |
| 14 | Breadboard / PCB | Prototype board | 1 | — | Component mounting |

## Pin Assignment

| ESP32 Pin | Connected To | Function |
|-----------|-------------|----------|
| GPIO 32 | X Left Limit Switch | X-axis left boundary detect |
| GPIO 33 | X Right Limit Switch | X-axis right boundary detect |
| GPIO 4 | Y Bottom Limit Switch | Y-axis bottom boundary detect |
| GPIO 5 | Y Top Limit Switch | Y-axis top boundary detect |
| GPIO 14 | DRV8833 XIN1 | X motor forward PWM |
| GPIO 12 | DRV8833 XIN2 | X motor reverse PWM |
| GPIO 27 | DRV8833 YIN1 | Y motor forward PWM |
| GPIO 26 | DRV8833 YIN2 | Y motor reverse PWM |
| GPIO 2 | DRV8833 SLEEP + LED | Driver enable / status LED |
| GPIO 13 | Emergency Stop Button | Hardware emergency cutoff |
