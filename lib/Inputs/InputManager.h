#pragma once
#include <Arduino.h>
#include <Encoder.h>
#include "InputEvent.h"
#include "Pot.h"

// Holds press/hold state tracking per encoder
struct ButtonState
{
    bool pressed = false;
    uint32_t pressTime = 0;
};

class InputManager
{
public:
    InputManager(Encoder **encList, uint8_t count, Pot *pot);
    void begin();
    bool poll(InputEvent &e); // returns true if event is produced

private:
    Encoder **encs;
    uint8_t count;
    Pot *pot;

    static const uint32_t SAVE_HOLD_TIME = 5000; // 5 seconds for save combo

    int32_t lastDet[5];
    ButtonState btn[5];
    uint8_t lastPot = 0;

    uint32_t saveComboStartTime = 0;
    bool saveComboActive = false;

    void checkEncoder(uint8_t i, InputEvent &e);
    void checkPot(InputEvent &e);

    InputAction actionFor(uint8_t encIndex, bool hold);
    InputAction toggleActionFor(uint8_t encIndex);
};
