#include <Arduino.h>

#include "led.h"

namespace
{
constexpr uint8_t pin = 10;
constexpr unsigned long disconnectedBlinkIntervalMs = 500;
constexpr unsigned long acknowledgementBlinkOnDurationMs = 50;
constexpr unsigned long defaultAcknowledgementBlinkOffDurationMs =
    acknowledgementBlinkOnDurationMs;
constexpr uint8_t acknowledgementBlinkCount = 2;
constexpr unsigned long acknowledgementTimeoutMs = 1000;

unsigned long buttonFlashStartedAt;
unsigned long buttonFlashDuration;
unsigned long acknowledgementStartedAt;
unsigned long acknowledgementBlinkOffDuration;
unsigned long statusChangedAt;
bool buttonFlashActive;
bool buttonAcknowledgementPending;
bool acknowledgementActive;
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
    buttonAcknowledgementPending = false;
    acknowledgementActive = false;
    acknowledgementBlinkOffDuration =
        defaultAcknowledgementBlinkOffDurationMs;
    statusChangedAt = millis();
}

void Led::activateFor(unsigned long durationMs)
{
    buttonFlashStartedAt = millis();
    buttonFlashDuration = durationMs;
    buttonFlashActive = true;
    buttonAcknowledgementPending = true;
    setOutput(!(connected && palettePatternActive));
}

void Led::acknowledge()
{
    const unsigned long now = millis();
    acknowledgementBlinkOffDuration =
        defaultAcknowledgementBlinkOffDurationMs;
    if (buttonAcknowledgementPending)
    {
        const unsigned long buttonFlashElapsedMs = now - buttonFlashStartedAt;
        if (buttonFlashElapsedMs >= buttonFlashDuration)
        {
            acknowledgementBlinkOffDuration =
                buttonFlashElapsedMs - buttonFlashDuration;
            acknowledgementStartedAt = now;
        }
        else
        {
            acknowledgementStartedAt =
                buttonFlashStartedAt + buttonFlashDuration +
                acknowledgementBlinkOffDuration;
        }
    }
    else
    {
        acknowledgementStartedAt = now;
    }

    buttonAcknowledgementPending = false;
    acknowledgementActive = true;
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
    const unsigned long acknowledgementCycleDurationMs =
        acknowledgementBlinkOnDurationMs + acknowledgementBlinkOffDuration;
    const unsigned long acknowledgementDurationMs =
        acknowledgementBlinkCount * acknowledgementBlinkOnDurationMs +
        (acknowledgementBlinkCount - 1) * acknowledgementBlinkOffDuration;

    if (buttonFlashActive &&
        now - buttonFlashStartedAt >= buttonFlashDuration)
    {
        buttonFlashActive = false;
    }

    if (buttonAcknowledgementPending &&
        now - buttonFlashStartedAt >= acknowledgementTimeoutMs)
    {
        buttonAcknowledgementPending = false;
    }

    const bool acknowledgementHasStarted =
        static_cast<long>(now - acknowledgementStartedAt) >= 0;
    const unsigned long acknowledgementElapsedMs =
        now - acknowledgementStartedAt;
    if (acknowledgementActive && acknowledgementHasStarted &&
        acknowledgementElapsedMs >= acknowledgementDurationMs)
    {
        acknowledgementActive = false;
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
    const bool acknowledgementBlinkIsOn =
        acknowledgementActive && acknowledgementHasStarted &&
        acknowledgementElapsedMs % acknowledgementCycleDurationMs <
            acknowledgementBlinkOnDurationMs;
    setOutput(
        (buttonFlashActive || acknowledgementBlinkIsOn)
            ? !statusOutputIsOn
            : statusOutputIsOn);
}
