#include <Arduino.h>

#include <Encoder.h>
#include <EncoderAdapter/PjrcEncoderAdapter.h>
#include <EventButton.h>
#include <EventEncoder.h>

#include "controls.h"

namespace
{
constexpr uint8_t buttonPins[] = {7, 20, 21, 2, 8, 9};
constexpr Controls::Action switchActions[] = {
    Controls::Action::BlinkerLeft,
    Controls::Action::HazardLights,
    Controls::Action::BlinkerRight,
    Controls::Action::ToggleCarMode,
    Controls::Action::NextPalette,
    Controls::Action::NextPattern,
};
constexpr uint8_t actionQueueSize = 16;
constexpr uint8_t encoder1A = 3;
constexpr uint8_t encoder1B = 0;
constexpr uint8_t encoder1Click = 1;
constexpr uint8_t encoder2A = 6;
constexpr uint8_t encoder2B = 4;
constexpr uint8_t encoder2Click = 5;

PjrcEncoderAdapter encoderAdapters[] = {
    {encoder1A, encoder1B},
    {encoder2A, encoder2B},
};
EventEncoder encoderInputs[] = {
    {&encoderAdapters[0]},
    {&encoderAdapters[1]},
};
EventButton encoderClickInputs[] = {
    {encoder1Click},
    {encoder2Click},
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
bool actionsEnabled = false;

bool queueAction(Controls::Action action)
{
    if (!actionsEnabled)
    {
        return false;
    }

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
    const size_t index = button.getInputId();
    if (index == 3)
    {
        if (event == InputEventType::LONG_CLICKED)
        {
            if (queueAction(Controls::Action::TurnOff))
            {
                Serial.println("Button 4 long clicked");
            }
        }
        else if (event == InputEventType::CLICKED)
        {
            if (queueAction(Controls::Action::ToggleCarMode))
            {
                Serial.println("Button 4 clicked");
            }
        }
        return;
    }

    if (event != InputEventType::PRESSED)
    {
        return;
    }

    if (queueAction(switchActions[index]))
    {
        Serial.printf("Button %u clicked\n", index + 1);
    }
}

void onEncoderEvent(InputEventType event, EventEncoder &encoder)
{
    if (event != InputEventType::CHANGED)
    {
        return;
    }

    const size_t index = encoder.getInputId();
    const int16_t increment = encoder.increment();
    const bool clockwise = increment > 0;
    const Controls::Action action =
        index == 0
            ? (clockwise ? Controls::Action::SpeedUp : Controls::Action::SpeedDown)
            : (clockwise ? Controls::Action::BrightnessUp : Controls::Action::BrightnessDown);
    const uint16_t actionCount = clockwise ? increment : -increment;

    for (uint16_t step = 0; step < actionCount; ++step)
    {
        if (!queueAction(action))
        {
            break;
        }
    }

    const char *direction = clockwise ? "clockwise" : "counterclockwise";
    Serial.printf("Rotary encoder %u rotated %s\n", index + 1, direction);
}

void onEncoderClickEvent(InputEventType event, EventButton &button)
{
    if (event == InputEventType::PRESSED)
    {
        Serial.printf("Rotary encoder %u clicked\n", button.getInputId() + 1);
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
        encoderInputs[index].begin();
        encoderInputs[index].setCallback(onEncoderEvent);

        encoderClickInputs[index].setInputId(index);
        encoderClickInputs[index].setDebounceInterval(30);
        encoderClickInputs[index].begin();
        encoderClickInputs[index].setCallback(onEncoderClickEvent);
    }
}

void Controls::setActionsEnabled(bool enabled)
{
    actionsEnabled = enabled;
    if (!enabled)
    {
        actionQueueHead = 0;
        actionQueueTail = 0;
        actionQueueCount = 0;
    }
}

bool Controls::pollAction(Action &action)
{
    for (EventEncoder &encoder : encoderInputs)
    {
        encoder.update();
    }

    for (EventButton &encoderClick : encoderClickInputs)
    {
        encoderClick.update();
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
