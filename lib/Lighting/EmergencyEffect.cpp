#include "EmergencyEffect.h"
#include <Arduino.h>

void EmergencyEffect::render(const LightingParams &P, const SpatialMap &map,
                             CRGB *mainLeds, uint16_t mainCount,
                             CRGB *detailLeds, uint16_t detailCount,
                             uint32_t now)
{
    // Speed controls rotation rate and fade speed
    float speedFactor = P.activeConfig->speed / 100.0f;

    // Update rotation angle for detail LEDs
    angle += speedFactor * 0.03f;
    if (angle > TWO_PI)
        angle -= TWO_PI;

    // Render detail LEDs: rotating blue/red split
    for (uint16_t i = 0; i < detailCount; i++)
    {
        const Vec3 &pos = map.pos(i);
        float ledAngle = atan2(pos.y, pos.x); // Calculate angle from x,y position

        // Determine if this LED is in blue or red half
        float relativeAngle = ledAngle - angle;
        if (relativeAngle < 0)
            relativeAngle += TWO_PI;

        if (relativeAngle < PI)
        {
            // Blue half
            detailLeds[i] = CRGB::Blue;
        }
        else
        {
            // Red half
            detailLeds[i] = CRGB::Red;
        }
    }

    // Main LEDs: smooth fade between blue and red
    // Update fade value based on speed
    int fadeStep = (int)(speedFactor * 2.0f);

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
