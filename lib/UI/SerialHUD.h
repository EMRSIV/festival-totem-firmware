#pragma once
#include <Arduino.h>
#include "LightingParams.h"
#include "EffectManager.h"

class SerialHUD
{
public:
    void begin()
    {
        lastPrint = 0;
        dirty = true;
    }

    void markDirty() { dirty = true; }

    void update(const LightingParams &P,
                const EffectManager &fx,
                uint32_t now)
    {
        if (!dirty)
            return;

        if (now - lastPrint < 100)
            return; // rate limit
        lastPrint = now;
        dirty = false;

        Serial.println("\n--- EMRSIV Lighting State ---");
        Serial.printf("Mode          : %d\n", (int)P.activeMode);
        Serial.printf("Effect        : %s\n", fx.activeName());
        Serial.printf("Main Color    : H=%3u S=%3u\n", P.mainHue(), P.mainSat());

        Serial.printf("Secondary     : %s\n", P.secondaryEnabled() ? "ON" : "OFF");
        Serial.printf("Sec Color     : H=%3u S=%3u\n", P.secondaryHue(), P.secondarySat());

        Serial.printf("Intensity     : %3u\n", P.intensity());
        Serial.printf("Speed         : %3u\n", P.speed());
        Serial.printf("Brightness    : %3u\n", P.brightness);
        if (P.strobeActive)
            Serial.print("(STROBE) ");
        Serial.println("-------------------------------");
    }

private:
    bool dirty = false;
    uint32_t lastPrint = 0;
};
