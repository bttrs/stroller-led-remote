#include "bluetooth.h"
#include "controls.h"
#include "led.h"

namespace
{
constexpr unsigned long buttonPressLedDurationMs = 25;

void sendAction(Controls::Action action)
{
    switch (action)
    {
    case Controls::Action::BlinkerLeft:
        Bluetooth::blinkerLeft();
        break;
    case Controls::Action::BlinkerRight:
        Bluetooth::blinkerRight();
        break;
    case Controls::Action::HazardLights:
        Bluetooth::hazardLights();
        break;
    case Controls::Action::ToggleAutoPattern:
        Bluetooth::toggleAutoPattern();
        break;
    case Controls::Action::ToggleAutoPalette:
        Bluetooth::toggleAutoPalette();
        break;
    case Controls::Action::NextPattern:
        Bluetooth::nextPattern();
        break;
    case Controls::Action::NextPalette:
        Bluetooth::nextPalette();
        break;
    case Controls::Action::TurnOff:
        Bluetooth::turnOff();
        break;
    case Controls::Action::ToggleCarMode:
        Bluetooth::toggleCarMode();
        break;
    }
}
}

void setup()
{
    Controls::initialize();
    Led::initialize();
    Bluetooth::initialize();
}

void loop()
{
    Controls::Action action;
    while (Controls::pollAction(action))
    {
        sendAction(action);
        Led::activateFor(buttonPressLedDurationMs);
    }

    Bluetooth::update();
    Led::setConnectionStatus(Bluetooth::isConnected());
    Led::update();
}
