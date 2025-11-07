#include "InputMapper.h"
#include <Arduino.h>

bool InputMapper::apply(const InputEvent &e, LightingParams &P)
{
    if (!P.activeConfig)
    {
        Serial.println("ERROR: activeConfig is null!");
        return false;
    }

    switch (e.action)
    {
    // Adjustments routed to activeConfig
    case InputAction::MainHueAdjust:
        P.activeConfig->mainHue += e.value;
        break;
    case InputAction::MainSatAdjust:
        P.activeConfig->mainSat = constrain(P.activeConfig->mainSat + e.value, 0, 255);
        break;

    case InputAction::SecondaryHueAdjust:
        P.activeConfig->secondaryHue += e.value;
        break;
    case InputAction::SecondarySatAdjust:
        P.activeConfig->secondarySat = constrain(P.activeConfig->secondarySat + e.value, 0, 255);
        break;

    case InputAction::IntensityAdjust:
        P.activeConfig->intensity = constrain(P.activeConfig->intensity + e.value, 0, 255);
        break;

    case InputAction::SpeedAdjust:
        P.activeConfig->speed = constrain(P.activeConfig->speed + e.value, 0, 255);
        break;

    // Effect switching (only in Default mode)
    case InputAction::EffectAdjust:
        if (P.activeMode == ConfigMode::Default)
        {
            P.effectID += e.value;
        }
        break;

    // Toggle secondary (only in Default mode)
    case InputAction::ToggleSecondaryColor:
        if (P.activeMode == ConfigMode::Default)
        {
            P.activeConfig->secondaryEnabled = !P.activeConfig->secondaryEnabled;
            Serial.printf("Secondary: %s\n", P.activeConfig->secondaryEnabled ? "ON" : "OFF");
        }
        break;

    // Mode switching events (handled in main.cpp)
    case InputAction::EnterSpecial1:
    case InputAction::ExitSpecial1:
    case InputAction::EnterSpecial2:
    case InputAction::EnterSpecial3:
    case InputAction::ExitSpecial3:
    case InputAction::SaveConfigs:
        // These are handled by main.cpp, not here
        return true;

    // Global settings
    case InputAction::BrightnessDirect:
        P.brightness = e.value;
        break;

    default:
        return false;
    }

    return true;
}
