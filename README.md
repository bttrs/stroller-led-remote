# Stroller LED Remote

Initial PlatformIO firmware project for an ESP32-C3 SuperMini. It initializes
two rotary encoders, six buttons, and one LED. Input handling is deliberately
left as an inert skeleton for future expansion.

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

## Layout

- `src/` - firmware sources
- `include/` - shared project headers
- `lib/` - project-specific libraries
- `test/` - PlatformIO tests

## Build

```sh
pio run -e esp32-c3-supermini
```
