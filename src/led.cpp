#include <Arduino.h>

#include "led.h"

namespace
{
constexpr uint8_t pin = 10;

unsigned long turnedOnAt;
unsigned long duration;
bool isOn;
} // namespace

void Led::initialize()
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    isOn = false;
}

void Led::activateFor(unsigned long durationMs)
{
    digitalWrite(pin, HIGH);
    turnedOnAt = millis();
    duration = durationMs;
    isOn = true;
}

void Led::update()
{
    if (isOn && millis() - turnedOnAt >= duration)
    {
        digitalWrite(pin, LOW);
        isOn = false;
    }
}
