#pragma once

namespace Controls
{
enum class Action
{
    BlinkerLeft,
    BlinkerRight,
    HazardLights,
    Police,
    ToggleAutoPattern,
    ToggleAutoPalette,
    NextPattern,
    NextPalette,
    SpeedUp,
    SpeedDown,
    BrightnessUp,
    BrightnessDown,
    Extra1,
    Extra2,
    TurnOff,
    ToggleCarMode,
};

void initialize();

// Enables or disables collecting actions from physical controls.
void setActionsEnabled(bool enabled);

// Returns one queued control action.
bool pollAction(Action &action);
} // namespace Controls
