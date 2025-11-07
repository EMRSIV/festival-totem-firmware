#pragma once
#include "Effect.h"
#include <math.h>

// Forward declare BPM range (defined in main.cpp)
#ifndef MIN_BPM
#define MIN_BPM 50.0f
#endif
#ifndef MAX_BPM
#define MAX_BPM 180.0f
#endif

class SphereEffect : public Effect
{
private:
    float minRadius = 0.0f;
    float maxRadius = 0.0f;
    float currentRadius = 0.0f;
    float phase = 0.0f;
    uint32_t lastUpdate = 0;
    bool initialized = false;

    // Calculate bounding box to find min/max radius
    void calculateRadiusBounds(const SpatialMap &S)
    {
        if (initialized)
            return;

        float minDist = 999999.0f;
        float maxDist = 0.0f;

        // Find center point (should be close to 0,0,0 but let's calculate it)
        float centerX = 0.0f, centerY = 0.0f, centerZ = 0.0f;

        for (uint16_t i = 0; i < S.count(); i++)
        {
            const Vec3 &pos = S.pos(i);
            centerX += pos.x;
            centerY += pos.y;
            centerZ += pos.z;
        }
        centerX /= S.count();
        centerY /= S.count();
        centerZ /= S.count();

        // Now find min/max distances from center
        for (uint16_t i = 0; i < S.count(); i++)
        {
            const Vec3 &pos = S.pos(i);
            float dx = pos.x - centerX;
            float dy = pos.y - centerY;
            float dz = pos.z - centerZ;
            float dist = sqrtf(dx * dx + dy * dy + dz * dz);

            if (dist < minDist)
                minDist = dist;
            if (dist > maxDist)
                maxDist = dist;
        }

        // Add small buffer to ensure full coverage
        minRadius = 0.0f;           // Start from center
        maxRadius = maxDist * 1.1f; // 10% extra to ensure full coverage

        initialized = true;
    }

public:
    void render(const LightingParams &P,
                const SpatialMap &S,
                CRGB *mainLeds, uint16_t nMain,
                CRGB *detailLeds, uint16_t nDetail,
                uint32_t nowMs) override
    {
        // Initialize radius bounds on first run
        calculateRadiusBounds(S);

        // Calculate delta time
        if (lastUpdate == 0)
            lastUpdate = nowMs;

        float deltaTime = (nowMs - lastUpdate) / 1000.0f;
        lastUpdate = nowMs;

        // Map speed (0-255) to BPM range (uses global MIN_BPM/MAX_BPM defines)
        float speedNorm = P.speed() / 255.0f;
        float bpm = MIN_BPM + speedNorm * (MAX_BPM - MIN_BPM);

        // Convert BPM to cycles per second
        float cyclesPerSecond = bpm / 60.0f;

        // Update phase (0 to 1 represents one full expansion cycle)
        phase += deltaTime * cyclesPerSecond;
        if (phase > 1.0f)
            phase -= 1.0f;

        // Calculate current radius (expands from min to max)
        currentRadius = minRadius + (maxRadius - minRadius) * phase;

        // Intensity controls the shell thickness (mapped from 0-1 to 0.1-2.0)
        float intensityNorm = P.intensity() / 255.0f;       // 0 to 1
        float shellThickness = 0.1f + intensityNorm * 1.9f; // 0.1 to 2.0 cm

        // Main LEDs: pulsing based on sphere phase
        uint8_t mainBrightness = (uint8_t)(255.0f * sinf(phase * PI));
        CRGB mainColor = CHSV(P.mainHue(), P.mainSat(), mainBrightness);
        fill_solid(mainLeds, nMain, mainColor);

        // Detail LEDs: sphere shell effect
        for (uint16_t i = 0; i < nDetail; i++)
        {
            const Vec3 &pos = S.pos(i);

            // Calculate distance from center
            float dist = sqrtf(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);

            // Calculate how close this LED is to the sphere shell
            float distFromShell = fabsf(dist - currentRadius);

            // Brightness falls off based on distance from shell
            float brightness = 1.0f - (distFromShell / shellThickness);
            brightness = fmaxf(0.0f, fminf(1.0f, brightness));

            // Apply smooth curve for nicer falloff
            brightness = brightness * brightness; // Quadratic falloff

            if (P.secondaryEnabled())
            {
                // Blend between main and secondary based on expansion phase
                CRGB color1 = CHSV(P.mainHue(), P.mainSat(), 255);
                CRGB color2 = CHSV(P.secondaryHue(), P.secondarySat(), 255);
                CRGB blendedColor = blend(color1, color2, phase * 255);

                // Apply brightness to blended color
                detailLeds[i] = blendedColor;
                detailLeds[i].nscale8((uint8_t)(brightness * 255));
            }
            else
            {
                // Single color mode
                uint8_t finalBrightness = (uint8_t)(brightness * 255);
                detailLeds[i] = CHSV(P.mainHue(), P.mainSat(), finalBrightness);
            }
        }
    }
};
