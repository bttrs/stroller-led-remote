# Stroller LED Remote

Firmware for an ESP32-C3 SuperMini with two rotary encoders, six buttons, and
one LED. Input handling uses the `InputEvents` library: each debounced button
press turns the LED on briefly, while rotary turns and clicks are reported to
the serial log only. The rotary encoders are KY-040 modules.

## Hardware pins

All encoder and button inputs use `INPUT_PULLUP`, so their switches should
connect the GPIO pin to ground when pressed.

| Device | Signal | GPIO |
| --- | --- | ---: |
| Encoder 1 | CLK | 3 |
| Encoder 1 | DT | 0 |
| Encoder 1 | SW | 1 |
| Encoder 2 | CLK | 6 |
| Encoder 2 | DT | 4 |
| Encoder 2 | SW | 5 |
| LED | Signal | 10 |
| Button 1 | | 7 |
| Button 2 | | 20 |
| Button 3 | | 21 |
| Button 4 | | 2 |
| Button 5 | | 8 |
| Button 6 | | 9 |

GPIO 2, 8, and 9 are ESP32-C3 strapping pins. Ensure their buttons do not hold
the pins low during reset or boot.

## Controls

| Input | Action |
| --- | --- |
| Button 1 | Left blinker |
| Button 2 | Hazard lights |
| Button 3 | Right blinker |
| Button 4 click / long click | Toggle car mode / turn LEDs off |
| Button 5 | Next palette |
| Button 6 | Next pattern |
| Encoder 1 clockwise / counterclockwise | Increase / decrease speed |
| Encoder 2 clockwise / counterclockwise | Increase / decrease brightness |
| Encoder 1 / 2 click | Serial log only |

The remote LED blinks while it is discovering or reconnecting to the stroller
and remains off after the command channel connects. Button activity briefly
lights it regardless of connection status.

## Layout

- `src/main.cpp` - application setup and coordination
- `src/controls.cpp`, `include/controls.h` - control initialization using
  `InputEvents`, debounced button actions, and rotary speed/brightness actions
- `src/led.cpp`, `include/led.h` - LED timing and output behavior
- `src/bluetooth.cpp`, `include/bluetooth.h` - BLE client that continuously
  discovers the `Led Stroller` peripheral by advertised name or command
  service, reconnects after a disconnect, and writes the stroller's commands
  to its command characteristic. Commands entered before connection are queued
  and sent without write responses every 15 ms.
- `lib/` - project-specific libraries
- `test/` - PlatformIO tests

## Build

```sh
pio run -e esp32-c3-supermini
```
