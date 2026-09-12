#pragma once

namespace Bluetooth
{
// Starts discovery for the Led Stroller BLE peripheral.
void initialize();

// Maintains the BLE connection and transmits queued commands.
void update();

bool isConnected();
bool isPalettePatternActive();

bool blinkerLeft();
bool blinkerRight();
bool hazardLights();
bool toggleAutoPattern();
bool toggleAutoPalette();
bool nextPattern();
bool nextPalette();
bool speedUp();
bool speedDown();
bool brightnessUp();
bool brightnessDown();
bool extra1();
bool extra2();
bool turnOff();
bool toggleCarMode();
} // namespace Bluetooth
