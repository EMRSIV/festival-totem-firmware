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

    // Configure secondary color brightness scaling
    void setSecondaryBrightnessRange(uint8_t minBrightness, uint8_t maxBrightness)
    {
        secondaryBrightnessMin = minBrightness;
        secondaryBrightnessMax = maxBrightness;
    }

    uint8_t getSecondaryBrightnessMin() const { return secondaryBrightnessMin; }
    uint8_t getSecondaryBrightnessMax() const { return secondaryBrightnessMax; }

    // Configure explosion height threshold (0.0 = bottom, 1.0 = top)
    void setExplosionHeightThreshold(float threshold)
    {
        explosionHeightThreshold = threshold;
    }

    float getExplosionHeightThreshold() const { return explosionHeightThreshold; }

private:
    EnergyBurstState state = EnergyBurstState::Inactive;
    uint32_t explosionStartTime = 0;
    float angle = 0.0f; // Current angle of spinning point

    // Secondary color brightness scaling (0-255)
    uint8_t secondaryBrightnessMin = 0;
    uint8_t secondaryBrightnessMax = 40;

    // Explosion height threshold (0.0 = bottom, 1.0 = top)
    float explosionHeightThreshold = 0.2f;

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
