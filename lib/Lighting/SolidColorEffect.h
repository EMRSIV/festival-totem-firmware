#pragma once
#include "Effect.h"

class SolidColorEffect : public Effect
{
public:
    void render(const LightingParams &P,
                const SpatialMap &,
                CRGB *mainLeds, uint16_t nMain,
                CRGB *detailLeds, uint16_t nDetail,
                uint32_t);
};
