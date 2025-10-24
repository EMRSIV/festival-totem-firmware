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
    bool holdActive = false;
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

    static const uint32_t HOLD_TIME = 250; // was 500

    int32_t lastDet[5];
    ButtonState btn[5];
    uint8_t lastPot = 0;

    void checkEncoder(uint8_t i, InputEvent &e);
    void checkPot(InputEvent &e);

    InputAction actionFor(uint8_t encIndex, bool hold);
    InputAction toggleActionFor(uint8_t encIndex);
};
