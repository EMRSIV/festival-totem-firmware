#pragma once
#include <FastLED.h>
#include "LightingParams.h"
#include "SpatialMap.h"

class Effect
{
public:
    virtual ~Effect() {}
    virtual void begin() {}
    virtual void render(const LightingParams &p,
                        const SpatialMap &s,
                        CRGB *mainLeds,
                        uint16_t mainCount,
                        CRGB *detailLeds,
                        uint16_t detailCount,
                        uint32_t nowMs) = 0;
};
