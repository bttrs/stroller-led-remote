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
    case Controls::Action::SpeedUp:
        Bluetooth::speedUp();
        break;
    case Controls::Action::SpeedDown:
        Bluetooth::speedDown();
        break;
    case Controls::Action::BrightnessUp:
        Bluetooth::brightnessUp();
        break;
    case Controls::Action::BrightnessDown:
        Bluetooth::brightnessDown();
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
    Serial.begin(115200);
    Serial.println("Stroller LED remote starting");
    Controls::initialize();
    Serial.println("Controls initialized");
    Led::initialize();
    Bluetooth::initialize();
    Serial.println("Bluetooth discovery started");
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
    Led::setPalettePatternStatus(Bluetooth::isPalettePatternActive());
    Led::update();
}
