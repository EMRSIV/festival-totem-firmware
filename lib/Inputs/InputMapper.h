#pragma once
#include "InputEvent.h"
#include "LightingParams.h"

class InputMapper
{
public:
    bool apply(const InputEvent &e, LightingParams &P);
};
