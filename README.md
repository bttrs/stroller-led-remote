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

## Layout

- `src/main.cpp` - application setup and coordination
- `src/controls.cpp`, `include/controls.h` - control initialization and
  debounced button press events
- `src/led.cpp`, `include/led.h` - LED timing and output behavior
- `src/bluetooth.cpp`, `include/bluetooth.h` - dormant BLE client that
  reconnects to `BLUETOOTH_TARGET_ADDRESS` when its `initialize()` and
  `update()` functions are later integrated. Set that build macro to the
  target MAC address; use `BLUETOOTH_TARGET_ADDRESS_TYPE` if it has a random
  address rather than the default public address. The client writes
  hyphenated command names to `FF00`/`FF01` by default and logs received
  acknowledgement notifications; configure the command and acknowledgement
  GATT UUIDs with the corresponding `BLUETOOTH_*_UUID` build macros.
- `lib/` - project-specific libraries
- `test/` - PlatformIO tests

## Build

```sh
pio run -e esp32-c3-supermini
```
