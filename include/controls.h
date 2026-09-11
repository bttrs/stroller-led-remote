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
    TurnOff,
    ToggleCarMode,
};

void initialize();

// Returns one queued button action.
bool pollAction(Action &action);
} // namespace Controls
