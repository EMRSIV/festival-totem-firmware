#pragma once
#include "Effect.h"
#include "SpatialMap.h"
#include "EffectConfig.h"

// Forward declare BPM range (defined in main.cpp)
#ifndef MIN_BPM
#define MIN_BPM 50.0f
#endif
#ifndef MAX_BPM
#define MAX_BPM 180.0f
#endif

class RainEffect : public Effect
{
public:
    void render(const LightingParams &P, const SpatialMap &map,
                CRGB *mainLeds, uint16_t mainCount,
                CRGB *detailLeds, uint16_t detailCount,
                uint32_t now) override;

    void reset();

private:
    // Raindrop tracking
    struct Raindrop
    {
        uint16_t stringIndex;   // Which string (0 to NUM_STRINGS-1)
        float height;           // Current normalized height (1.0 = top, 0.0 = bottom)
        bool useSecondaryColor; // true if this drop uses secondary color
        bool active;
        uint8_t mainLedPhase; // 0 = not in main LEDs, 1 = LED 2, 2 = LED 1, 3 = entering strings
    };

    static const uint8_t MAX_RAINDROPS = 64; // Maximum simultaneous raindrops
    Raindrop raindrops[MAX_RAINDROPS];

    uint32_t lastSpawnTime = 0;
    uint32_t lastUpdateTime = 0;
};
