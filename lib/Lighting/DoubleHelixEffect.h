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

private:
    float phase = 0.0f;      // Accumulated phase to prevent jumping
    uint32_t lastUpdate = 0; // Last update time for delta calculation
};
