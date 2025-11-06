#include "EmergencyEffect.h"
#include <Arduino.h>

void EmergencyEffect::render(const LightingParams &P, const SpatialMap &map,
                             CRGB *mainLeds, uint16_t mainCount,
                             CRGB *detailLeds, uint16_t detailCount,
                             uint32_t now)
{
    // Speed controls rotation rate and fade speed
    // Minimum speed of 20% so it always moves
    float speedFactor = 0.2f + (P.activeConfig->speed / 100.0f) * 0.8f;

    // Update rotation angle for detail LEDs (doubled speed)
    angle += speedFactor * 0.06f;
    if (angle > TWO_PI)
        angle -= TWO_PI;

    // Render detail LEDs: rotating blue/red split per string with linear fade
    const uint8_t numStrings = map.segments() * 2;          // 16 strings (2 per segment)
    const uint8_t ledsPerString = detailCount / numStrings; // 15 LEDs per string

    for (uint16_t i = 0; i < detailCount; i++)
    {
        // Determine which string this LED belongs to
        uint8_t stringIndex = i / ledsPerString;

        // Calculate the angular position of this string (0 to TWO_PI)
        // Strings are arranged in a circle: 0 = front, increasing clockwise
        float stringAngle = (stringIndex * TWO_PI) / numStrings;

        // Calculate relative angle to rotation
        float relativeAngle = stringAngle - angle;
        if (relativeAngle < 0)
            relativeAngle += TWO_PI;

        // Small fade zone at the two transition points (0° and 180°)
        const float fadeZone = 0.17f; // ~10 degrees on each side of transition

        // Determine color based on position with fade at boundaries
        if (relativeAngle < fadeZone)
        {
            // Near 0° - fade from red to blue
            float fadeRatio = relativeAngle / fadeZone; // 0.0 to 1.0
            detailLeds[i] = blend(CRGB::Red, CRGB::Blue, fadeRatio * 255);
        }
        else if (relativeAngle < PI - fadeZone)
        {
            // Pure blue (front half, away from boundaries)
            detailLeds[i] = CRGB::Blue;
        }
        else if (relativeAngle < PI + fadeZone)
        {
            // Near 180° - fade from blue to red
            // Normalize to 0.0-1.0 across the entire 2*fadeZone width
            float fadeRatio = (relativeAngle - (PI - fadeZone)) / (2.0f * fadeZone);
            detailLeds[i] = blend(CRGB::Blue, CRGB::Red, fadeRatio * 255);
        }
        else if (relativeAngle < TWO_PI - fadeZone)
        {
            // Pure red (back half, away from boundaries)
            detailLeds[i] = CRGB::Red;
        }
        else
        {
            // Near 360° (approaching 0°) - fade from red to blue
            float fadeRatio = (relativeAngle - (TWO_PI - fadeZone)) / fadeZone; // 0.0 to 1.0
            detailLeds[i] = blend(CRGB::Red, CRGB::Blue, fadeRatio * 255);
        }
    }

    // Main LEDs: smooth fade between blue and red
    // Update fade value based on speed (always at least 1 step)
    int fadeStep = max(1, (int)(speedFactor * 2.0f));

    if (fadeDirection)
    {
        mainFade += fadeStep;
        if (mainFade >= 255)
        {
            mainFade = 255;
            fadeDirection = false;
        }
    }
    else
    {
        mainFade -= fadeStep;
        if (mainFade <= 0)
        {
            mainFade = 0;
            fadeDirection = true;
        }
    }

    // Blend between blue and red
    CRGB blue = CRGB::Blue;
    CRGB red = CRGB::Red;

    for (uint16_t i = 0; i < mainCount; i++)
    {
        mainLeds[i] = blend(blue, red, mainFade);
    }
}
