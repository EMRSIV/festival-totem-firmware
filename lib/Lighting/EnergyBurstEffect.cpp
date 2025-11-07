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

    // Clear all droplets
    for (uint8_t i = 0; i < MAX_DROPLETS; i++)
    {
        droplets[i].active = false;
    }
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

        // Update spinning angle based on speed with minimum rotation
        // Speed range: 0-255 maps to 0.5-5.0 rotation factor (never stops)
        float speedFactor = 0.5f + (P.activeConfig->speed / 255.0f) * 4.5f;
        float angleIncrement = speedFactor * 0.05f;
        float previousAngle = angle;
        angle += angleIncrement;
        if (angle > TWO_PI)
            angle -= TWO_PI;

        // Main color for spinning point, detail color for background and droplets
        CRGB mainColor = CHSV(P.activeConfig->mainHue, P.activeConfig->mainSat, 255);
        CRGB detailColor = CHSV(P.activeConfig->secondaryHue, P.activeConfig->secondarySat, 255);

        // Calculate secondary brightness based on intensity
        // At intensity=0: use secondaryBrightnessMin
        // At intensity=255: use secondaryBrightnessMax
        float intensityRatio = P.activeConfig->intensity / 255.0f;
        uint8_t secondaryBrightness = secondaryBrightnessMin +
                                      (uint8_t)((secondaryBrightnessMax - secondaryBrightnessMin) * intensityRatio);

        CRGB dimColor = detailColor;
        dimColor.nscale8(secondaryBrightness);

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

        // Calculate rotation period (time for one full rotation in milliseconds)
        // angle increments by angleIncrement per frame at ~60fps
        // Full rotation = TWO_PI radians, so period = (TWO_PI / angleIncrement) * frame_time
        // Assuming ~16.67ms per frame (60fps)
        float rotationPeriod = (TWO_PI / angleIncrement) * 16.67f;

        // Calculate droplet fall rate per frame (should take one full rotation to fall)
        // If droplet falls from 1.0 to 0.0 over rotationPeriod, then per frame (16.67ms):
        float dropletFallRate = 1.0f / (rotationPeriod / 16.67f);

        // Find the string closest to the spinning angle
        // With map.segments() segments and 2 strings per segment
        const uint8_t NUM_STRINGS = map.segments() * 2;             // e.g., 8 * 2 = 16 strings total
        const uint16_t LEDS_PER_STRING = detailCount / NUM_STRINGS; // e.g., 240 / 16 = 15 LEDs per string

        // Calculate which string the angle points to
        float stringAngleStep = TWO_PI / NUM_STRINGS;
        uint8_t targetString = (uint8_t)(angle / stringAngleStep) % NUM_STRINGS;

        // Find the LED at the target height on the target string (spinning point)
        uint16_t stringStart = targetString * LEDS_PER_STRING;
        uint16_t stringEnd = stringStart + LEDS_PER_STRING;

        int spinningPointLED = -1;
        float closestHeightDiff = 999999.0f;

        for (uint16_t i = stringStart; i < stringEnd; i++)
        {
            const Vec3 &pos = map.pos(i);
            float ledHeight = pos.z;
            float normalizedHeight = (ledHeight - minHeight) / (maxHeight - minHeight);

            float heightDiff = abs(normalizedHeight - heightRatio);

            if (heightDiff < closestHeightDiff)
            {
                closestHeightDiff = heightDiff;
                spinningPointLED = i;
            }
        }

        // Check if we crossed a string boundary (spawn new droplet)
        uint8_t previousString = (uint8_t)(previousAngle / stringAngleStep) % NUM_STRINGS;
        if (previousString != targetString && spinningPointLED >= 0)
        {
            // Only spawn droplets if we're not at the bottom layer
            // At the bottom, droplets would just overlap with the background
            if (heightRatio > 0.1f) // Need some height to spawn droplets
            {
                // Spawn a new droplet at the spinning point location
                for (uint8_t d = 0; d < MAX_DROPLETS; d++)
                {
                    if (!droplets[d].active)
                    {
                        droplets[d].startLED = spinningPointLED;
                        droplets[d].progress = 0.0f; // Start at top
                        droplets[d].active = true;
                        break;
                    }
                }
            }
        }

        // Update and render droplets
        for (uint8_t d = 0; d < MAX_DROPLETS; d++)
        {
            if (!droplets[d].active)
                continue;

            // Advance droplet progress based on speed
            droplets[d].progress += dropletFallRate;

            // Droplet takes exactly one rotation period to fall from top to bottom
            if (droplets[d].progress >= 1.0f)
            {
                // Droplet reached bottom, deactivate
                droplets[d].active = false;
                continue;
            }

            // Get the string this droplet is on
            uint16_t startLED = droplets[d].startLED;
            uint16_t dropletString = startLED / LEDS_PER_STRING;
            uint16_t dropletStringStart = dropletString * LEDS_PER_STRING;

            // Get start height (normalized)
            float startHeight = map.pos(startLED).z;
            float startNormHeight = (startHeight - minHeight) / (maxHeight - minHeight);

            // Calculate current height (fall from startNormHeight to 0.0)
            float currentNormHeight = startNormHeight * (1.0f - droplets[d].progress);

            // Find the LED closest to this height on the same string
            int dropletLED = -1;
            float closestDropletHeightDiff = 999999.0f;

            for (uint16_t i = dropletStringStart; i < dropletStringStart + LEDS_PER_STRING; i++)
            {
                const Vec3 &pos = map.pos(i);
                float ledHeight = pos.z;
                float normalizedHeight = (ledHeight - minHeight) / (maxHeight - minHeight);

                float heightDiff = abs(normalizedHeight - currentNormHeight);

                if (heightDiff < closestDropletHeightDiff)
                {
                    closestDropletHeightDiff = heightDiff;
                    dropletLED = i;
                }
            }

            // Render the droplet
            if (dropletLED >= 0)
            {
                detailLeds[dropletLED] = mainColor;
            }
        }

        // Render the spinning point on top
        if (spinningPointLED >= 0)
        {
            detailLeds[spinningPointLED] = mainColor;
        }

        // Render background LEDs with interpolation (below spinning point)
        for (uint16_t i = 0; i < detailCount; i++)
        {
            // Skip if already rendered by droplet or spinning point
            if (detailLeds[i].r != 0 || detailLeds[i].g != 0 || detailLeds[i].b != 0)
                continue;

            const Vec3 &pos = map.pos(i);
            float ledHeight = pos.z;
            float normalizedHeight = (ledHeight - minHeight) / (maxHeight - minHeight);

            if (normalizedHeight < heightRatio)
            {
                // LEDs strictly below spinning point: interpolate brightness from 0 (at border) to dimmed (fully below)
                float distanceFromBorder = heightRatio - normalizedHeight;
                float fadeRatio = distanceFromBorder / heightRatio; // 0.0 at border, 1.0 at bottom

                // Clamp to ensure we stay in valid range
                if (fadeRatio > 1.0f)
                    fadeRatio = 1.0f;
                if (fadeRatio < 0.0f)
                    fadeRatio = 0.0f;

                // Apply linear fade: 0% at border to secondaryBrightness at bottom
                detailLeds[i] = detailColor;
                uint8_t scaleFactor = (uint8_t)(fadeRatio * secondaryBrightness);
                detailLeds[i].nscale8(scaleFactor);
            }
        }

        // Main LEDs off during buildup
        fill_solid(mainLeds, mainCount, CRGB::Black);
    }
}
