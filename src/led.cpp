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
bool palettePatternActive;
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
    palettePatternActive = false;
    disconnectedBlinkIsOn = false;
    buttonFlashActive = false;
    statusChangedAt = millis();
}

void Led::activateFor(unsigned long durationMs)
{
    buttonFlashStartedAt = millis();
    buttonFlashDuration = durationMs;
    buttonFlashActive = true;
    setOutput(!(connected && palettePatternActive));
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

void Led::setPalettePatternStatus(bool isPalettePatternActive)
{
    palettePatternActive = isPalettePatternActive;
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

    const bool statusOutputIsOn =
        (connected && palettePatternActive) ||
        (!connected && disconnectedBlinkIsOn);
    setOutput(buttonFlashActive ? !statusOutputIsOn : statusOutputIsOn);
}
