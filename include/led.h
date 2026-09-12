#pragma once

#include <Arduino.h>

namespace Led
{
void initialize();
void activateFor(unsigned long durationMs);
void acknowledge();
void setConnectionStatus(bool connected);
void setPalettePatternStatus(bool isPalettePatternActive);
void update();
} // namespace Led
