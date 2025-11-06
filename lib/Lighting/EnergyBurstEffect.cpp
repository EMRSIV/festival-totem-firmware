#include "EnergyBurstEffect.h"
#include <Arduino.h>

void EnergyBurstEffect::setState(EnergyBurstState newState)
{
    if (state != newState)
    {
        state = newState;
        if (state == EnergyBurstState::Exploding)
        {
            explosionStartTime = millis();
        }
    }
}

void EnergyBurstEffect::reset()
{
    state = EnergyBurstState::Inactive;
    angle = 0.0f;
}

void EnergyBurstEffect::render(const LightingParams &P, const SpatialMap &map,
                               CRGB *mainLeds, uint16_t mainCount,
                               CRGB *detailLeds, uint16_t detailCount,
                               uint32_t now)
{
    if (state == EnergyBurstState::Inactive)
    {
        return; // Nothing to render
    }

    if (state == EnergyBurstState::Exploding)
    {
        // Fast alternating main LEDs
        uint32_t elapsed = now - explosionStartTime;

        if (elapsed >= EXPLOSION_DURATION)
        {
            // Auto-exit after 2 seconds (will be handled by main.cpp)
            return;
        }

        // Fast toggle (every 50ms)
        bool toggle = (elapsed / 50) % 2 == 0;

        mainLeds[0] = toggle ? CRGB::White : CRGB::Black;
        mainLeds[1] = toggle ? CRGB::Black : CRGB::White;

        // Detail LEDs off during explosion
        fill_solid(detailLeds, detailCount, CRGB::Black);

        return;
    }

    if (state == EnergyBurstState::BuildingUp)
    {
        // Get intensity (0-255 maps to 0.0-1.0 height)
        float heightRatio = P.activeConfig->intensity / 255.0f;

        // Update spinning angle based on speed
        float speedFactor = P.activeConfig->speed / 50.0f;
        angle += speedFactor * 0.05f; // Adjust rotation rate
        if (angle > TWO_PI)
            angle -= TWO_PI;

        // Main color from config
        CRGB mainColor = CHSV(P.activeConfig->mainHue, P.activeConfig->mainSat, 255);
        CRGB dimColor = mainColor;
        dimColor.nscale8(128); // 50% brightness

        // Clear LEDs
        fill_solid(detailLeds, detailCount, CRGB::Black);

        // Find min/max heights once
        float minHeight = map.pos(0).z;
        float maxHeight = map.pos(0).z;
        for (uint16_t j = 1; j < detailCount; j++)
        {
            float h = map.pos(j).z;
            if (h < minHeight)
                minHeight = h;
            if (h > maxHeight)
                maxHeight = h;
        }

        // Find the spinning point position
        for (uint16_t i = 0; i < detailCount; i++)
        {
            const Vec3 &pos = map.pos(i);
            float ledAngle = atan2(pos.y, pos.x); // Calculate angle from position
            float ledHeight = pos.z;              // Height is z-coordinate

            float normalizedHeight = (ledHeight - minHeight) / (maxHeight - minHeight);

            // Check if this LED is the spinning point
            float angleDiff = abs(ledAngle - angle);
            if (angleDiff > PI)
                angleDiff = TWO_PI - angleDiff;

            // If we're at the spinning point's angle and height
            if (angleDiff < 0.2f && abs(normalizedHeight - heightRatio) < 0.05f)
            {
                detailLeds[i] = mainColor; // Full brightness at spinning point
            }
            // LEDs below the spinning point
            else if (normalizedHeight < heightRatio)
            {
                detailLeds[i] = dimColor; // 50% brightness
            }
        }

        // Main LEDs off during buildup
        fill_solid(mainLeds, mainCount, CRGB::Black);
    }
}
