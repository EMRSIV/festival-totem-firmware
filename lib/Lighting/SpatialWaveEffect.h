#pragma once
#include "Effect.h"
#include <math.h>

class SpatialWaveEffect : public Effect
{
private:
    float phase = 0.0f;
    uint32_t lastUpdate = 0;

public:
    void render(const LightingParams &P,
                const SpatialMap &S,
                CRGB *mainLeds, uint16_t nMain,
                CRGB *detailLeds, uint16_t nDetail,
                uint32_t nowMs) override
    {
        // Calculate delta time for smooth animation
        if (lastUpdate == 0)
            lastUpdate = nowMs;

        float deltaTime = (nowMs - lastUpdate) / 1000.0f;
        lastUpdate = nowMs;

        // Accumulate phase based on speed
        float speedFactor = P.speed() / 255.0f;
        phase += deltaTime * speedFactor * 5.0f; // 5.0 multiplier for visible speed

        // Keep phase in reasonable range
        if (phase > TWO_PI * 100.0f)
            phase -= TWO_PI * 100.0f;

        // Main brightness pulse
        uint8_t pulse = (sin(phase) * 0.5f + 0.5f) * P.intensity();
        fill_solid(mainLeds, nMain, CHSV(P.mainHue(), P.mainSat(), pulse));

        for (uint16_t i = 0; i < nDetail; i++)
        {
            const Vec3 &v = S.pos(i);
            float wave = sin((v.z * 0.07f) + phase);
            wave = (wave * 0.5f + 0.5f); // map to 0..1

            uint8_t bri = wave * P.intensity();

            CHSV c = P.secondaryEnabled()
                         ? CHSV(P.secondaryHue(), P.secondarySat(), bri)
                         : CHSV(P.mainHue(), P.mainSat(), bri);

            detailLeds[i] = c;
        }
    }
};
