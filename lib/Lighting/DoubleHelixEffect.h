#pragma once
#include "Effect.h"

class DoubleHelixEffect : public Effect
{
public:
    const char *name() const { return "DoubleHelix"; }

    void render(const LightingParams &P,
                const SpatialMap &M,
                CRGB *mainLeds, uint16_t nMain,
                CRGB *detailLeds, uint16_t nDetail,
                uint32_t nowMs) override;
};
