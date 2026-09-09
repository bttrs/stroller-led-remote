#include <Arduino.h>

namespace Pins
{
constexpr uint8_t encoder1A = 0;
constexpr uint8_t encoder1B = 1;
constexpr uint8_t encoder1Click = 3;

constexpr uint8_t encoder2A = 4;
constexpr uint8_t encoder2B = 5;
constexpr uint8_t encoder2Click = 6;

constexpr uint8_t led = 10;

constexpr uint8_t buttons[] = {7, 20, 21, 2, 8, 9};
} // namespace Pins

struct RotaryEncoderPins
{
    uint8_t a;
    uint8_t b;
    uint8_t click;
};

constexpr RotaryEncoderPins encoders[] = {
    {Pins::encoder1A, Pins::encoder1B, Pins::encoder1Click},
    {Pins::encoder2A, Pins::encoder2B, Pins::encoder2Click},
};

void initializeControls()
{
    for (const RotaryEncoderPins &encoder : encoders)
    {
        pinMode(encoder.a, INPUT_PULLUP);
        pinMode(encoder.b, INPUT_PULLUP);
        pinMode(encoder.click, INPUT_PULLUP);
    }

    for (uint8_t button : Pins::buttons)
    {
        pinMode(button, INPUT_PULLUP);
    }
}

void pollControls()
{
    // Placeholder for future encoder and button event handling.
}

void setup()
{
    initializeControls();

    pinMode(Pins::led, OUTPUT);
    digitalWrite(Pins::led, LOW);
}

void loop()
{
    pollControls();
}
