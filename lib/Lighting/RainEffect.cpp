#include "RainEffect.h"
#include <Arduino.h>

void RainEffect::reset()
{
    // Deactivate all raindrops
    for (uint8_t i = 0; i < MAX_RAINDROPS; i++)
    {
        raindrops[i].active = false;
    }
    lastSpawnTime = 0;
    lastUpdateTime = 0;
}

void RainEffect::render(const LightingParams &P, const SpatialMap &map,
                        CRGB *mainLeds, uint16_t mainCount,
                        CRGB *detailLeds, uint16_t detailCount,
                        uint32_t now)
{
    // Clear all LEDs (no background)
    fill_solid(mainLeds, mainCount, CRGB::Black);
    fill_solid(detailLeds, detailCount, CRGB::Black);

    // Get parameters
    // Intensity: controls spawn rate (0-255 maps to very sparse to very dense)
    // Speed: maps to BPM (MIN_BPM to MAX_BPM), where one beat = one drop from top to bottom
    uint8_t intensity = P.activeConfig->intensity;
    uint8_t speed = P.activeConfig->speed;

    // Map speed (0-255) to BPM range
    float speedNorm = speed / 255.0f;
    float bpm = MIN_BPM + speedNorm * (MAX_BPM - MIN_BPM);

    // Calculate drop duration: one beat = one complete drop cycle
    // BPM = beats per minute, so beat duration in ms = 60000 / BPM
    float beatDurationMs = 60000.0f / bpm;

    CRGB mainColor = CHSV(P.activeConfig->mainHue, P.activeConfig->mainSat, 255);
    CRGB secondaryColor = CHSV(P.activeConfig->secondaryHue, P.activeConfig->secondarySat, 255);

    // Calculate number of strings
    const uint8_t NUM_STRINGS = map.segments() * 2; // e.g., 8 * 2 = 16 strings
    const uint16_t LEDS_PER_STRING = detailCount / NUM_STRINGS;

    // Initialize if first run
    if (lastUpdateTime == 0)
    {
        lastUpdateTime = now;
        lastSpawnTime = now;
    }

    // Calculate time deltas
    uint32_t deltaTime = now - lastUpdateTime;
    lastUpdateTime = now;

    // Spawn new raindrops based on intensity
    // Intensity 0 = spawn rarely, Intensity 255 = spawn very frequently
    // Map intensity to spawn interval: 0 → 500ms, 255 → 20ms
    uint32_t spawnInterval = 500 - (intensity * 480 / 255);

    if (now - lastSpawnTime >= spawnInterval)
    {
        lastSpawnTime = now;

        // Try to spawn a new raindrop
        for (uint8_t i = 0; i < MAX_RAINDROPS; i++)
        {
            if (!raindrops[i].active)
            {
                raindrops[i].stringIndex = random(NUM_STRINGS);
                raindrops[i].height = 0.5f;                          // Start at top (0.5 height unit for upper LED phase)
                raindrops[i].useSecondaryColor = (random(100) < 25); // 25% chance for secondary color
                raindrops[i].mainLedPhase = 0;                       // Start above main LEDs
                raindrops[i].active = true;
                break;
            }
        }
    }

    // Calculate fall speed based on BPM
    // One beat = complete drop cycle (top to bottom)
    // Total height to traverse: 3 main LED phases (each 0.5 height units) + 1.0 string height = 2.5 units
    // Main LEDs are 50% of string height, so 3 phases × 0.5 = 1.5 units for main LEDs
    // Fall rate = total distance / beat duration
    const float MAIN_LED_HEIGHT_PER_PHASE = 0.5f; // Upper LEDs are 50% of string height
    const float STRING_HEIGHT = 1.0f;
    const float TOTAL_DROP_HEIGHT = (3.0f * MAIN_LED_HEIGHT_PER_PHASE) + STRING_HEIGHT; // 1.5 + 1.0 = 2.5 units
    float fallRatePerMs = TOTAL_DROP_HEIGHT / beatDurationMs;
    float fallDistance = fallRatePerMs * deltaTime;

    // Find min/max heights for normalization
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

    // Update and render all active raindrops
    for (uint8_t i = 0; i < MAX_RAINDROPS; i++)
    {
        if (!raindrops[i].active)
            continue;

        Raindrop &drop = raindrops[i];

        // Choose color for this drop
        // If secondary is disabled, all drops use main color
        CRGB dropColor = (P.activeConfig->secondaryEnabled && drop.useSecondaryColor)
                             ? secondaryColor
                             : mainColor;

        // Phase 0-3: Drop falling through main LEDs (above the strings)
        if (drop.mainLedPhase < 3)
        {
            // Each main LED phase represents 0.5 height units (50% of string height)
            // This ensures the upper LEDs take half the time as the strings for accurate BPM
            drop.height -= fallDistance;

            if (drop.height <= 0.0f)
            {
                // Move to next phase
                drop.mainLedPhase++;
                drop.height = MAIN_LED_HEIGHT_PER_PHASE; // Reset height for next phase

                if (drop.mainLedPhase >= 3)
                {
                    // Now entering the strings (full 1.0 height unit)
                    drop.height = STRING_HEIGHT;
                }
            }

            // Render on main LEDs based on phase
            if (drop.mainLedPhase == 1)
            {
                // In main LED 2 (index 1)
                mainLeds[1] = dropColor;
            }
            else if (drop.mainLedPhase == 2)
            {
                // In main LED 1 (index 0)
                mainLeds[0] = dropColor;
            }

            continue; // Don't render on detail LEDs yet
        }

        // Phase 3+: Drop is falling down the strings
        drop.height -= fallDistance;

        if (drop.height <= 0.0f)
        {
            // Drop reached bottom, deactivate
            drop.active = false;
            continue;
        }

        // Find the LED string this drop is on
        uint16_t stringStart = drop.stringIndex * LEDS_PER_STRING;
        uint16_t stringEnd = stringStart + LEDS_PER_STRING;

        // Find the two LEDs that form this 2-LED drop at current height
        int led1 = -1;
        int led2 = -1;
        float closestHeightDiff1 = 999999.0f;
        float closestHeightDiff2 = 999999.0f;

        // Find LED closest to current height
        for (uint16_t j = stringStart; j < stringEnd; j++)
        {
            const Vec3 &pos = map.pos(j);
            float ledHeight = pos.z;
            float normalizedHeight = (ledHeight - minHeight) / (maxHeight - minHeight);
            float heightDiff = abs(normalizedHeight - drop.height);

            if (heightDiff < closestHeightDiff1)
            {
                closestHeightDiff1 = heightDiff;
                led1 = j;
            }
        }

        // Find LED just below led1 for the 2-LED drop
        if (led1 >= 0)
        {
            float led1Height = (map.pos(led1).z - minHeight) / (maxHeight - minHeight);

            for (uint16_t j = stringStart; j < stringEnd; j++)
            {
                if (j == led1)
                    continue;

                const Vec3 &pos = map.pos(j);
                float ledHeight = pos.z;
                float normalizedHeight = (ledHeight - minHeight) / (maxHeight - minHeight);

                // Look for LED below led1
                if (normalizedHeight < led1Height)
                {
                    float heightDiff = abs(normalizedHeight - (drop.height - 0.1f)); // Offset for 2nd LED
                    if (heightDiff < closestHeightDiff2)
                    {
                        closestHeightDiff2 = heightDiff;
                        led2 = j;
                    }
                }
            }
        }

        // Render the 2-LED drop
        if (led1 >= 0)
        {
            detailLeds[led1] = dropColor;
        }
        if (led2 >= 0)
        {
            detailLeds[led2] = dropColor;
        }
    }
}
