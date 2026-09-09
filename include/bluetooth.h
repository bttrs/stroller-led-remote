#pragma once

namespace Bluetooth
{
// Initializes the BLE client for BLUETOOTH_TARGET_ADDRESS.
void initialize();

// Attempts to reconnect at a fixed interval while the target is unavailable.
void update();

bool isConnected();

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
