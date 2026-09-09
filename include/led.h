#pragma once

#include <Arduino.h>

namespace Led
{
void initialize();
void activateFor(unsigned long durationMs);
void update();
} // namespace Led
