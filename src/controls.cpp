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

constexpr uint8_t switches[] = {7, 20, 21, 2, 8, 9};
constexpr Controls::Action switchActions[] = {
    Controls::Action::BlinkerLeft,
    Controls::Action::HazardLights,
    Controls::Action::BlinkerRight,
    Controls::Action::ToggleCarMode,
    Controls::Action::TurnOff,
    Controls::Action::NextPattern,
};
constexpr unsigned long debounceDurationMs = 30;
constexpr uint8_t actionQueueSize = 16;

struct RotaryEncoderPins
{
    uint8_t a;
    uint8_t b;
    uint8_t click;
};

constexpr RotaryEncoderPins encoders[] = {
    {encoder1A, encoder1B, encoder1Click},
    {encoder2A, encoder2B, encoder2Click},
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
ButtonState encoderClickStates[sizeof(encoders) / sizeof(encoders[0])];
EncoderState encoderStates[sizeof(encoders) / sizeof(encoders[0])];
Controls::Action actionQueue[actionQueueSize];
uint8_t actionQueueHead = 0;
uint8_t actionQueueTail = 0;
uint8_t actionQueueCount = 0;

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

uint8_t readEncoderPosition(const RotaryEncoderPins &encoder)
{
    return (digitalRead(encoder.a) == HIGH ? 0b10 : 0) |
           (digitalRead(encoder.b) == HIGH ? 0b01 : 0);
}

bool hasDebouncedPress(uint8_t pin, ButtonState &state, unsigned long now)
{
    const bool rawPressed = digitalRead(pin) == LOW;

    if (rawPressed != state.rawPressed)
    {
        state.rawPressed = rawPressed;
        state.rawStateChangedAt = now;
    }

    if (state.pressed == state.rawPressed ||
        now - state.rawStateChangedAt < debounceDurationMs)
    {
        return false;
    }

    state.pressed = state.rawPressed;
    return state.pressed;
}

void logEncoderInput(
    const RotaryEncoderPins &encoder,
    EncoderState &state,
    ButtonState &clickState,
    size_t index,
    unsigned long now)
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

    if (transitionDeltas[transition] != 0)
    {
        state.transitionCount += transitionDeltas[transition];
        if (state.transitionCount >= 4 || state.transitionCount <= -4)
        {
            const char *direction =
                state.transitionCount > 0 ? "clockwise" : "counterclockwise";
            state.transitionCount = 0;
            Serial.printf("Rotary encoder %u rotated %s\n", index + 1, direction);
        }
    }
    else if (position != ((transition >> 2) & 0b11))
    {
        state.transitionCount = 0;
    }

    if (hasDebouncedPress(encoder.click, clickState, now))
    {
        Serial.printf("Rotary encoder %u clicked\n", index + 1);
    }
}
} // namespace

void Controls::initialize()
{
    for (const RotaryEncoderPins &encoder : encoders)
    {
        pinMode(encoder.a, INPUT_PULLUP);
        pinMode(encoder.b, INPUT_PULLUP);
        pinMode(encoder.click, INPUT_PULLUP);
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
        encoderStates[index] = {
            readEncoderPosition(encoders[index]),
            0,
        };
        const bool clickPressed = digitalRead(encoders[index].click) == LOW;
        encoderClickStates[index] = {
            clickPressed,
            clickPressed,
            now,
        };
    }
}

bool Controls::pollAction(Action &action)
{
    const unsigned long now = millis();
    for (size_t index = 0; index < sizeof(encoders) / sizeof(encoders[0]); ++index)
    {
        logEncoderInput(
            encoders[index],
            encoderStates[index],
            encoderClickStates[index],
            index,
            now);
    }

    for (size_t index = 0; index < sizeof(switches) / sizeof(switches[0]); ++index)
    {
        ButtonState &state = switchStates[index];
        if (hasDebouncedPress(switches[index], state, now))
        {
            if (queueAction(switchActions[index]))
            {
                Serial.printf("Button %u clicked\n", index + 1);
            }
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
