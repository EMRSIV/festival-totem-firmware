#include "InputMapper.h"
#include <Arduino.h>

bool InputMapper::apply(const InputEvent &e, LightingParams &P)
{
    switch (e.action)
    {
    case InputAction::MainHueAdjust:
        P.mainHue += e.value * 3;
        break;
    case InputAction::MainSatAdjust:
        P.mainSat = constrain(P.mainSat + e.value * 5, 0, 255);
        break;

    case InputAction::SecondaryHueAdjust:
        P.secondaryHue += e.value * 3;
        break;
    case InputAction::SecondarySatAdjust:
        P.secondarySat = constrain(P.secondarySat + e.value * 5, 0, 255);
        break;

    case InputAction::IntensityAdjust:
        P.intensity = constrain(P.intensity + e.value * 4, 0, 255);
        break;
    case InputAction::Special1Adjust:
        P.special1 = e.value > 0; // TODO: refine to param later
        break;

    case InputAction::SpeedAdjust:
        P.speed = constrain(P.speed + e.value * 4, 0, 255);
        break;
    case InputAction::Special2Adjust:
        P.special2 = e.value > 0;
        break;

    case InputAction::EffectAdjust:
        P.effectID += e.value;
        break;

    case InputAction::ToggleSecondaryColor:
        P.secondaryEnabled = !P.secondaryEnabled;
        Serial.printf("Secondary: %s\n", P.secondaryEnabled ? "ON" : "OFF");
        break;

    case InputAction::StrobeHoldStart:
        P.strobe = true;
        break;

    case InputAction::StrobeHoldEnd:
        P.strobe = false;
        break;

    case InputAction::StrobeSpeedAdjust:
        P.strobeSpeed = constrain((int)P.strobeSpeed + e.value * 6, 0, 255); // step size 6; tune to taste
        break;

    case InputAction::BrightnessDirect:
        P.brightness = e.value;
        break;

    default:
        return false;
    }

    return true;
}
