#include "controls.h"
#include "led.h"

namespace
{
constexpr unsigned long buttonPressLedDurationMs = 1000;
}

void setup()
{
    Controls::initialize();
    Led::initialize();
}

void loop()
{
    if (Controls::pollActivity())
    {
        Led::activateFor(buttonPressLedDurationMs);
    }

    Led::update();
}
