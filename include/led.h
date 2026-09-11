#pragma once

#include <Arduino.h>

namespace Led
{
void initialize();
void activateFor(unsigned long durationMs);
void setConnectionStatus(bool connected);
void update();
} // namespace Led
