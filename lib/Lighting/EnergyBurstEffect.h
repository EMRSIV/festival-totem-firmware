#pragma once
#include "Effect.h"
#include "SpatialMap.h"
#include "EffectConfig.h"

class EnergyBurstEffect : public Effect
{
public:
    void render(const LightingParams &P, const SpatialMap &map,
                CRGB *mainLeds, uint16_t mainCount,
                CRGB *detailLeds, uint16_t detailCount,
                uint32_t now) override;

    void setState(EnergyBurstState state);
    EnergyBurstState getState() const { return state; }

    void reset();

private:
    EnergyBurstState state = EnergyBurstState::Inactive;
    uint32_t explosionStartTime = 0;
    float angle = 0.0f; // Current angle of spinning point

    // Droplet tracking
    struct Droplet
    {
        uint16_t startLED; // LED index where droplet started
        float progress;    // 0.0 to 1.0, how far down the droplet has fallen
        bool active;
    };
    static const uint8_t MAX_DROPLETS = 32; // Enough for multiple rotations
    Droplet droplets[MAX_DROPLETS];

    static const uint32_t EXPLOSION_DURATION = 2000; // 2 seconds
};
