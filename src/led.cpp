#include <Arduino.h>

#include "led.h"

namespace
{
constexpr uint8_t pin = 10;
constexpr unsigned long disconnectedBlinkIntervalMs = 500;

unsigned long buttonFlashStartedAt;
unsigned long buttonFlashDuration;
unsigned long statusChangedAt;
bool buttonFlashActive;
bool connected;
bool disconnectedBlinkIsOn;
bool outputIsOn;

void setOutput(bool isOn)
{
    if (outputIsOn == isOn)
    {
        return;
    }

    digitalWrite(pin, isOn ? HIGH : LOW);
    outputIsOn = isOn;
}
} // namespace

void Led::initialize()
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    outputIsOn = false;
    connected = false;
    disconnectedBlinkIsOn = false;
    buttonFlashActive = false;
    statusChangedAt = millis();
}

void Led::activateFor(unsigned long durationMs)
{
    buttonFlashStartedAt = millis();
    buttonFlashDuration = durationMs;
    buttonFlashActive = true;
    setOutput(true);
}

void Led::setConnectionStatus(bool isConnected)
{
    if (connected == isConnected)
    {
        return;
    }

    connected = isConnected;
    disconnectedBlinkIsOn = false;
    statusChangedAt = millis();
}

void Led::update()
{
    const unsigned long now = millis();
    if (buttonFlashActive &&
        now - buttonFlashStartedAt >= buttonFlashDuration)
    {
        buttonFlashActive = false;
    }

    if (!connected &&
        now - statusChangedAt >= disconnectedBlinkIntervalMs)
    {
        disconnectedBlinkIsOn = !disconnectedBlinkIsOn;
        statusChangedAt = now;
    }

    setOutput(buttonFlashActive || (!connected && disconnectedBlinkIsOn));
}
