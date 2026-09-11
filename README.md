# Stroller LED Remote

Firmware for an ESP32-C3 SuperMini with two rotary encoders, six buttons, and
one LED. Each debounced button or encoder-click press, and every completed
encoder detent, turns the LED on for one second. Holding a switch generates
only one press event; another event requires releasing it and pressing again.

## Hardware pins

All encoder and button inputs use `INPUT_PULLUP`, so their switches should
connect the GPIO pin to ground when pressed.

| Device | Signal | GPIO |
| --- | --- | ---: |
| Encoder 1 | A | 0 |
| Encoder 1 | B | 1 |
| Encoder 1 | Click | 3 |
| Encoder 2 | A | 4 |
| Encoder 2 | B | 5 |
| Encoder 2 | Click | 6 |
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
| Button 4 | Toggle car mode |
| Button 5 | Turn LEDs off |
| Button 6 | Next pattern |
| Encoder 1 turn / click | Next pattern / toggle automatic patterns |
| Encoder 2 turn / click | Next palette / toggle automatic palettes |

The remote LED blinks while it is discovering or reconnecting to the stroller
and remains off after the command channel connects. Button activity briefly
lights it regardless of connection status.

## Layout

- `src/main.cpp` - application setup and coordination
- `src/controls.cpp`, `include/controls.h` - control initialization and
  debounced button, encoder-click, and encoder-detent actions
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
