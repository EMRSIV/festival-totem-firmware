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
        lastDet[i] = encs[i]->getScaledDetentCount();
    }
    if (pot)
        pot->begin();
}

// Maps enc rotation + (hold state) → action
InputAction InputManager::actionFor(uint8_t enc, bool hold)
{
    switch (enc)
    {
    case 0: // Enc1
        return hold ? InputAction::MainSatAdjust : InputAction::MainHueAdjust;
    case 1: // Enc2
        return hold ? InputAction::SecondarySatAdjust : InputAction::SecondaryHueAdjust;
    case 2: // Enc3
        return InputAction::EffectAdjust;
    case 3: // Enc4
        return InputAction::IntensityAdjust;
    case 4: // Enc5
        return InputAction::SpeedAdjust;
    default:
        return InputAction::None;
    }
}

// Maps button press events → toggles
InputAction InputManager::toggleActionFor(uint8_t enc)
{
    switch (enc)
    {
    case 1: // Enc2
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

    // Check for save combo (Enc4 + Enc5 both pressed for 5s)
    bool enc4Pressed = (encs[3]->getButtonStateRaw() == LOW);
    bool enc5Pressed = (encs[4]->getButtonStateRaw() == LOW);

    if (enc4Pressed && enc5Pressed)
    {
        if (!saveComboActive)
        {
            saveComboActive = true;
            saveComboStartTime = now;
        }
        else if (now - saveComboStartTime >= SAVE_HOLD_TIME)
        {
            // Trigger save (only once)
            e.action = InputAction::SaveConfigs;
            e.value = 1;
            saveComboActive = false; // Prevent repeated triggers
            return true;
        }
    }
    else
    {
        saveComboActive = false;
    }

    // Check encoders
    for (uint8_t i = 0; i < count; i++)
    {
        encs[i]->update();

        bool sw = (encs[i]->getButtonStateRaw() == LOW);

        // Press detection (fires immediately)
        if (sw && !btn[i].pressed)
        {
            btn[i].pressed = true;
            btn[i].pressTime = now;

            // Mode activation on press
            if (i == 3) // Enc4: Enter Special2
            {
                e.action = InputAction::EnterSpecial2;
                e.value = 1;
                return true;
            }
            else if (i == 4) // Enc5: Enter Special1 (strobe)
            {
                e.action = InputAction::EnterSpecial1;
                e.value = 1;
                return true;
            }
            else if (i == 2) // Enc3: Enter Special3 (emergency)
            {
                e.action = InputAction::EnterSpecial3;
                e.value = 1;
                return true;
            }
        }

        // Release detection
        if (!sw && btn[i].pressed)
        {
            // Mode deactivation on release
            if (i == 4) // Enc5: Exit Special1
            {
                e.action = InputAction::ExitSpecial1;
                e.value = 0;
                btn[i] = {};
                return true;
            }
            else if (i == 2) // Enc3: Exit Special3
            {
                e.action = InputAction::ExitSpecial3;
                e.value = 0;
                btn[i] = {};
                return true;
            }

            // Other buttons: short-press toggles (non-mode buttons)
            if (i != 3 && i != 4 && i != 2) // Not Enc3, Enc4, or Enc5
            {
                InputAction a = toggleActionFor(i);
                if (a != InputAction::None)
                {
                    e.action = a;
                    e.value = 1;
                    btn[i] = {};
                    return true;
                }
            }

            btn[i] = {};
        }

        // Rotational deltas
        int32_t det = encs[i]->getScaledDetentCount();
        int delta = det - lastDet[i];
        if (delta != 0)
        {
            lastDet[i] = det;
            InputAction a = actionFor(i, btn[i].pressed);
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
