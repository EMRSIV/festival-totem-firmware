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

        // Speed controls animation rate with minimum to ensure always moving
        // Map speed 0-255 to speed range 0.5x to 10.0x (never stops)
        float speedNorm = P.speed() / 255.0f;
        float speedFactor = 0.5f + speedNorm * 9.5f; // Min 0.5x, Max 10.0x
        phase += deltaTime * speedFactor;

        // Keep phase in reasonable range
        if (phase > TWO_PI * 100.0f)
            phase -= TWO_PI * 100.0f;

        // Intensity controls wavelength (spatial frequency)
        // Map intensity 0-255 to wavelength 5cm-60cm
        // Wavelength in cm, need to convert to spatial frequency
        float intensityNorm = P.intensity() / 255.0f;
        float wavelengthCm = 5.0f + intensityNorm * 55.0f; // 5cm to 60cm
        // Spatial frequency = 2*PI / wavelength (in cm)
        // v.z is in cm, so frequency directly relates to wavelength
        float spatialFrequency = (2.0f * PI) / wavelengthCm;

        // Fixed brightness for visibility
        uint8_t fixedBrightness = 255;

        // Main brightness pulse
        uint8_t pulse = (sin(phase) * 0.5f + 0.5f) * fixedBrightness;
        fill_solid(mainLeds, nMain, CHSV(P.mainHue(), P.mainSat(), pulse));

        for (uint16_t i = 0; i < nDetail; i++)
        {
            const Vec3 &v = S.pos(i);
            // Apply spatial frequency based on wavelength
            float wave = sin(v.z * spatialFrequency + phase);
            wave = (wave * 0.5f + 0.5f); // map to 0..1

            uint8_t bri = wave * fixedBrightness;

            CHSV c = P.secondaryEnabled()
                         ? CHSV(P.secondaryHue(), P.secondarySat(), bri)
                         : CHSV(P.mainHue(), P.mainSat(), bri);

            detailLeds[i] = c;
        }
    }
};
