#pragma once
#include "Effect.h"
#include "SpatialMap.h"

class EmergencyEffect : public Effect
{
public:
    void render(const LightingParams &P, const SpatialMap &map,
                CRGB *mainLeds, uint16_t mainCount,
                CRGB *detailLeds, uint16_t detailCount,
                uint32_t now) override;

private:
    float angle = 0.0f;        // Current rotation angle for blue/red split
    uint8_t mainFade = 0;      // Fade value for main LEDs (0=blue, 255=red)
    bool fadeDirection = true; // true = increasing toward red
};
