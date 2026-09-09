#pragma once

namespace Bluetooth
{
// Initializes the BLE client for BLUETOOTH_TARGET_ADDRESS.
void initialize();

// Attempts to reconnect at a fixed interval and transmits queued commands.
void update();

bool isConnected();

// Queue a command for non-blocking transmission. Returns false if unavailable
// or if the command queue is full.
bool increaseBrightness();
bool decreaseBrightness();
bool increaseSpeed();
bool decreaseSpeed();
bool blinkerLeft();
bool blinkerRight();
bool nextPattern();
bool nextPalette();
bool nextMode();
} // namespace Bluetooth
