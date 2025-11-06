#pragma once
#include "Effect.h"
#include <math.h>

class SpatialWaveEffect : public Effect
{
public:
    void render(const LightingParams &P,
                const SpatialMap &S,
                CRGB *mainLeds, uint16_t nMain,
                CRGB *detailLeds, uint16_t nDetail,
                uint32_t nowMs) override
    {
        // Main brightness pulse
        uint8_t pulse = (sin(nowMs * 0.005f * (float)P.speed()) * 0.5f + 0.5f) * P.intensity();
        fill_solid(mainLeds, nMain, CHSV(P.mainHue(), P.mainSat(), pulse));

        for (uint16_t i = 0; i < nDetail; i++)
        {
            const Vec3 &v = S.pos(i);
            float wave = sin((v.z * 0.07f) + (nowMs * 0.005f * (float)P.speed()));
            wave = (wave * 0.5f + 0.5f); // map to 0..1

            uint8_t bri = wave * P.intensity();

            CHSV c = P.secondaryEnabled()
                         ? CHSV(P.secondaryHue(), P.secondarySat(), bri)
                         : CHSV(P.mainHue(), P.mainSat(), bri);

            detailLeds[i] = c;
        }
    }
};
