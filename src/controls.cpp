#include <Arduino.h>

#include "controls.h"

namespace
{
constexpr uint8_t encoder1A = 0;
constexpr uint8_t encoder1B = 1;
constexpr uint8_t encoder1Click = 3;

constexpr uint8_t encoder2A = 4;
constexpr uint8_t encoder2B = 5;
constexpr uint8_t encoder2Click = 6;

constexpr uint8_t switches[] = {7, 20, 21, 2, 8, 9, encoder1Click, encoder2Click};
constexpr Controls::Action switchActions[] = {
    Controls::Action::BlinkerLeft,
    Controls::Action::HazardLights,
    Controls::Action::BlinkerRight,
    Controls::Action::ToggleCarMode,
    Controls::Action::TurnOff,
    Controls::Action::NextPattern,
    Controls::Action::ToggleAutoPattern,
    Controls::Action::ToggleAutoPalette,
};
constexpr unsigned long debounceDurationMs = 30;
constexpr uint8_t actionQueueSize = 16;

struct RotaryEncoderPins
{
    uint8_t a;
    uint8_t b;
};

constexpr RotaryEncoderPins encoders[] = {
    {encoder1A, encoder1B},
    {encoder2A, encoder2B},
};
constexpr Controls::Action encoderActions[] = {
    Controls::Action::NextPattern,
    Controls::Action::NextPalette,
};

struct ButtonState
{
    bool rawPressed;
    bool pressed;
    unsigned long rawStateChangedAt;
};

struct EncoderState
{
    uint8_t previousPosition;
    int8_t transitionCount;
};

ButtonState switchStates[sizeof(switches) / sizeof(switches[0])];
EncoderState encoderStates[sizeof(encoders) / sizeof(encoders[0])];
Controls::Action actionQueue[actionQueueSize];
uint8_t actionQueueHead = 0;
uint8_t actionQueueTail = 0;
uint8_t actionQueueCount = 0;

uint8_t readEncoderPosition(const RotaryEncoderPins &encoder)
{
    return (digitalRead(encoder.a) == HIGH ? 0b10 : 0) |
           (digitalRead(encoder.b) == HIGH ? 0b01 : 0);
}

bool queueAction(Controls::Action action)
{
    if (actionQueueCount == actionQueueSize)
    {
        Serial.println("Control action dropped: queue is full");
        return false;
    }

    actionQueue[actionQueueTail] = action;
    actionQueueTail = (actionQueueTail + 1) % actionQueueSize;
    ++actionQueueCount;
    return true;
}

bool pollEncoder(const RotaryEncoderPins &encoder, EncoderState &state)
{
    const uint8_t position = readEncoderPosition(encoder);
    const uint8_t transition = (state.previousPosition << 2) | position;
    state.previousPosition = position;

    constexpr int8_t transitionDeltas[] = {
        0, -1, 1, 0,
        1, 0, 0, -1,
        -1, 0, 0, 1,
        0, 1, -1, 0,
    };

    state.transitionCount += transitionDeltas[transition];
    if (state.transitionCount >= 4 || state.transitionCount <= -4)
    {
        state.transitionCount = 0;
        return true;
    }

    return false;
}
} // namespace

void Controls::initialize()
{
    for (const RotaryEncoderPins &encoder : encoders)
    {
        pinMode(encoder.a, INPUT_PULLUP);
        pinMode(encoder.b, INPUT_PULLUP);
    }

    const unsigned long now = millis();
    for (size_t index = 0; index < sizeof(switches) / sizeof(switches[0]); ++index)
    {
        pinMode(switches[index], INPUT_PULLUP);

        const bool pressed = digitalRead(switches[index]) == LOW;
        switchStates[index] = {pressed, pressed, now};
    }

    for (size_t index = 0; index < sizeof(encoders) / sizeof(encoders[0]); ++index)
    {
        encoderStates[index] = {readEncoderPosition(encoders[index]), 0};
    }
}

bool Controls::pollAction(Action &action)
{
    const unsigned long now = millis();
    for (size_t index = 0; index < sizeof(switches) / sizeof(switches[0]); ++index)
    {
        ButtonState &state = switchStates[index];
        const bool rawPressed = digitalRead(switches[index]) == LOW;

        if (rawPressed != state.rawPressed)
        {
            state.rawPressed = rawPressed;
            state.rawStateChangedAt = now;
        }

        if (state.pressed != state.rawPressed &&
            now - state.rawStateChangedAt >= debounceDurationMs)
        {
            state.pressed = state.rawPressed;
            if (state.pressed)
            {
                queueAction(switchActions[index]);
            }
        }
    }

    for (size_t index = 0; index < sizeof(encoders) / sizeof(encoders[0]); ++index)
    {
        if (pollEncoder(encoders[index], encoderStates[index]))
        {
            queueAction(encoderActions[index]);
        }
    }

    if (actionQueueCount == 0)
    {
        return false;
    }

    action = actionQueue[actionQueueHead];
    actionQueueHead = (actionQueueHead + 1) % actionQueueSize;
    --actionQueueCount;
    return true;
}
