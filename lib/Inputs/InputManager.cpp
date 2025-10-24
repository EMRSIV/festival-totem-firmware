#include "InputManager.h"

InputManager::InputManager(Encoder **encList, uint8_t count, Pot *pot)
    : encs(encList), count(count), pot(pot)
{
    for (uint8_t i = 0; i < count; i++)
    {
        lastDet[i] = 0;
        btn[i] = {};
    }
}

void InputManager::begin()
{
    for (uint8_t i = 0; i < count; ++i)
    {
        encs[i]->begin();
        lastDet[i] = encs[i]->getDetentCount();
    }
    if (pot)
        pot->begin();
}

// Maps enc rotation + (hold state) → action
InputAction InputManager::actionFor(uint8_t enc, bool hold)
{
    switch (enc)
    {
    case 0:
        return hold ? InputAction::MainSatAdjust : InputAction::MainHueAdjust;
    case 1:
        return hold ? InputAction::SecondarySatAdjust : InputAction::SecondaryHueAdjust;
    case 2:
        return InputAction::EffectAdjust;
    case 3:
        return hold ? InputAction::Special1Adjust : InputAction::IntensityAdjust;
    case 4:
        return hold ? InputAction::StrobeSpeedAdjust : InputAction::SpeedAdjust;
    default:
        return InputAction::None;
    }
}

// Maps button press events → toggles
InputAction InputManager::toggleActionFor(uint8_t enc)
{
    switch (enc)
    {
    case 1:
        return InputAction::ToggleSecondaryColor;
    default:
        return InputAction::None;
    }
}

bool InputManager::poll(InputEvent &e)
{
    e.action = InputAction::None;
    e.value = 0;

    uint32_t now = millis();

    // Check encoders first
    for (uint8_t i = 0; i < count; i++)
    {
        encs[i]->update();

        bool sw = (encs[i]->getButtonStateRaw() == LOW);

        // Press detection (fires immediately)
        if (sw && !btn[i].pressed)
        {
            btn[i].pressed = true;
            btn[i].pressTime = now;

            // Enc5: strobe ON instantly when pressed
            if (i == 4)
            {
                e.action = InputAction::StrobeHoldStart;
                e.value = 1;
                return true;
            }
        }

        // Release detection (fires instantly)
        if (!sw && btn[i].pressed)
        {
            // Strobe OFF on release
            if (i == 4)
            {
                e.action = InputAction::StrobeHoldEnd;
                e.value = 0;
                btn[i] = {};
                return true;
            }

            // Other buttons: short-press toggles
            InputAction a = toggleActionFor(i);
            if (a != InputAction::None)
            {
                e.action = a;
                e.value = 1;
                btn[i] = {};
                return true;
            }

            btn[i] = {};
        }

        // Rotational deltas
        int32_t det = encs[i]->getDetentCount();
        int delta = det - lastDet[i];
        if (delta != 0)
        {
            lastDet[i] = det;
            InputAction a = actionFor(i, btn[i].pressed); // ✅ FIX: pressed = hold
            e.action = a;
            e.value = delta;
            return true;
        }
    }

    // Pot polling
    if (pot)
    {
        pot->update();
        uint8_t v = pot->value8();
        if (abs((int)v - (int)lastPot) >= 2)
        {
            lastPot = v;
            e.action = InputAction::BrightnessDirect;
            e.value = v;
            return true;
        }
    }

    return false;
}
