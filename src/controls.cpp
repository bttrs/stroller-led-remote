#include <Arduino.h>

#include <Encoder.h>
#include <EncoderAdapter/PjrcEncoderAdapter.h>
#include <EventButton.h>
#include <EventEncoderButton.h>

#include "controls.h"

namespace
{
constexpr uint8_t buttonPins[] = {7, 20, 21, 2, 8, 9};
constexpr Controls::Action switchActions[] = {
    Controls::Action::BlinkerLeft,
    Controls::Action::HazardLights,
    Controls::Action::BlinkerRight,
    Controls::Action::ToggleCarMode,
    Controls::Action::TurnOff,
    Controls::Action::NextPattern,
};
constexpr uint8_t actionQueueSize = 16;
constexpr uint8_t encoder1A = 3;
constexpr uint8_t encoder1B = 0;
constexpr uint8_t encoder1Click = 1;
constexpr uint8_t encoder2A = 4;
constexpr uint8_t encoder2B = 5;
constexpr uint8_t encoder2Click = 6;

PjrcEncoderAdapter encoderAdapters[] = {
    {encoder1A, encoder1B},
    {encoder2A, encoder2B},
};
EventEncoderButton encoderInputs[] = {
    {&encoderAdapters[0], encoder1Click},
    {&encoderAdapters[1], encoder2Click},
};
EventButton buttonInputs[] = {
    {buttonPins[0]},
    {buttonPins[1]},
    {buttonPins[2]},
    {buttonPins[3]},
    {buttonPins[4]},
    {buttonPins[5]},
};
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

void onButtonEvent(InputEventType event, EventButton &button)
{
    if (event != InputEventType::PRESSED)
    {
        return;
    }

    const size_t index = button.getInputId();
    if (queueAction(switchActions[index]))
    {
        Serial.printf("Button %u clicked\n", index + 1);
    }
}

void onEncoderEvent(InputEventType event, EventEncoderButton &encoder)
{
    const size_t index = encoder.getInputId();
    if (event == InputEventType::CHANGED)
    {
        const char *direction = encoder.increment() > 0 ? "clockwise" : "counterclockwise";
        Serial.printf("Rotary encoder %u rotated %s\n", index + 1, direction);
    }
    else if (event == InputEventType::CLICKED)
    {
        Serial.printf("Rotary encoder %u clicked\n", index + 1);
    }
}
} // namespace

void Controls::initialize()
{
    for (size_t index = 0; index < sizeof(buttonInputs) / sizeof(buttonInputs[0]); ++index)
    {
        buttonInputs[index].setInputId(index);
        buttonInputs[index].setDebounceInterval(30);
        buttonInputs[index].begin();
        buttonInputs[index].setCallback(onButtonEvent);
    }

    for (size_t index = 0; index < sizeof(encoderInputs) / sizeof(encoderInputs[0]); ++index)
    {
        encoderInputs[index].setInputId(index);
        encoderInputs[index].setDebounceInterval(30);
        encoderInputs[index].begin();
        encoderInputs[index].setCallback(onEncoderEvent);
    }
}

bool Controls::pollAction(Action &action)
{
    for (EventEncoderButton &encoder : encoderInputs)
    {
        encoder.update();
    }

    for (EventButton &button : buttonInputs)
    {
        button.update();
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
