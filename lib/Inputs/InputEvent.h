#pragma once
#include <stdint.h>

enum class InputAction
{
    None = 0,

    // Primary adjustments
    MainHueAdjust,
    MainSatAdjust, // Enc1 hold
    SecondaryHueAdjust,
    SecondarySatAdjust, // Enc2 hold
    IntensityAdjust,
    Special1Adjust, // Enc4 hold
    SpeedAdjust,
    Special2Adjust, // Enc5 hold

    // Strobe
    StrobeHoldStart,   // begin hold
    StrobeHoldEnd,     // release
    StrobeSpeedAdjust, // Enc5 rotate while holding

    // Toggles
    ToggleSecondaryColor, // Enc2 press

    // Navigator
    EffectAdjust, // Enc3 rotate

    // Direct mappings
    BrightnessDirect, // Pot scaled 0..255
};

struct InputEvent
{
    InputAction action = InputAction::None;
    int value = 0; // positive or negative delta, or absolute
};
