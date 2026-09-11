#pragma once

namespace Controls
{
enum class Action
{
    BlinkerLeft,
    BlinkerRight,
    HazardLights,
    ToggleAutoPattern,
    ToggleAutoPalette,
    NextPattern,
    NextPalette,
    SpeedUp,
    SpeedDown,
    BrightnessUp,
    BrightnessDown,
    TurnOff,
    ToggleCarMode,
};

void initialize();

// Returns one queued control action.
bool pollAction(Action &action);
} // namespace Controls
