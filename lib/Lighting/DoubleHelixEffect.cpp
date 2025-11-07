#include "DoubleHelixEffect.h"
#include <FastLED.h>
#include <math.h>

void DoubleHelixEffect::render(const LightingParams &P,
                               const SpatialMap &M,
                               CRGB *mainLeds, uint16_t nMain,
                               CRGB *detailLeds, uint16_t nDetail,
                               uint32_t now)
{
    // Calculate delta time for smooth animation without jumping
    if (lastUpdate == 0)
        lastUpdate = now;

    float deltaTime = (now - lastUpdate) / 1000.0f; // Convert to seconds
    lastUpdate = now;

    // Speed: map from 0-255 to a reasonable animation speed
    // Minimum speed of 0.1x, maximum of 5x
    float speedFactor = 0.1f + (P.speed() / 255.0f) * 4.9f;

    // Accumulate phase based on speed - this prevents jumping when speed changes
    // Doubled speed multiplier (4.0f instead of 2.0f)
    phase += deltaTime * speedFactor * 4.0f;

    // Keep phase in reasonable range to avoid float precision issues
    if (phase > TWO_PI * 100.0f)
        phase -= TWO_PI * 100.0f;

    // Intensity controls the interpolation zone width (inverted)
    // High intensity (255) = sharp transition (small blend zone = 0.01)
    // Low intensity (0) = wide blend zone (large blend = 1.0)
    float blendWidth = 1.0f - (P.intensity() / 255.0f);
    // Ensure minimum blend width for visibility
    blendWidth = blendWidth * 0.99f + 0.01f;

    CRGB pri = CHSV(P.mainHue(), P.mainSat(), 255);
    CRGB sec = CHSV(P.secondaryHue(), P.secondarySat(), 255);

    // When secondary is disabled, use black instead of main color
    if (!P.secondaryEnabled())
        sec = CRGB::Black;

    // ---- MAIN ELEMENT (2 LEDs) ----
    for (int i = 0; i < nMain; i++)
    {
        Vec3 p = M.pos(i);
        float ang = atan2f(p.y, p.x);
        float wave = sinf(ang * 2.0f + phase); // -1.0 to 1.0

        if (blendWidth < 0.02f)
        {
            // Sharp transition when intensity is very high
            mainLeds[i] = (wave > 0) ? pri : sec;
        }
        else
        {
            // Smooth blend based on intensity (inverted)
            // Map wave from [-1, 1] to blend ratio [0, 1]
            float blendRatio = (wave / blendWidth) * 0.5f + 0.5f;
            blendRatio = constrain(blendRatio, 0.0f, 1.0f);
            mainLeds[i] = blend(sec, pri, blendRatio * 255);
        }
    }

    // ---- DETAIL STRIP (3D double helix) ----
    for (int i = 0; i < nDetail; i++)
    {
        Vec3 p = M.pos(i);
        float ang = atan2f(p.y, p.x);
        float wave = sinf(ang * 2.0f + p.z * 0.12f + phase); // -1.0 to 1.0

        if (blendWidth < 0.02f)
        {
            // Sharp transition when intensity is very high
            detailLeds[i] = (wave > 0) ? pri : sec;
        }
        else
        {
            // Smooth blend based on intensity (inverted)
            // Map wave from [-1, 1] to blend ratio [0, 1]
            float blendRatio = (wave / blendWidth) * 0.5f + 0.5f;
            blendRatio = constrain(blendRatio, 0.0f, 1.0f);
            detailLeds[i] = blend(sec, pri, blendRatio * 255);
        }
    }
}
