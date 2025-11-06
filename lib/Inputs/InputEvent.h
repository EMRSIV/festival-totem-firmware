#pragma once
#include <stdint.h>

enum class InputAction
{
    None = 0,

    // Primary adjustments (routed to activeConfig)
    MainHueAdjust,
    MainSatAdjust,
    SecondaryHueAdjust,
    SecondarySatAdjust,
    IntensityAdjust,
    SpeedAdjust,

    // Mode switching
    EnterSpecial1, // Enc5 press: Activate strobe
    ExitSpecial1,  // Enc5 release: Deactivate strobe
    EnterSpecial2, // Enc4 press: Start/advance energy burst
    EnterSpecial3, // Enc3 press: Activate emergency lights
    ExitSpecial3,  // Enc3 release: Deactivate emergency lights

    // Special combo
    SaveConfigs, // Enc4 + Enc5 held 5s: Save all configs to NVS

    // Toggles (mode-dependent)
    ToggleSecondaryColor, // Enc2 press (only in Default mode)

    // Navigator (only in Default mode)
    EffectAdjust, // Enc3 rotate

    // Direct mappings
    BrightnessDirect, // Pot scaled 0..255
};

struct InputEvent
{
    InputAction action = InputAction::None;
    int value = 0; // positive or negative delta, or absolute
};
