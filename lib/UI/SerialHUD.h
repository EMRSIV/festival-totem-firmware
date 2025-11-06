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

        // Show mode and effect name
        const char *modeName = "Default";
        if (P.activeMode == ConfigMode::Special1_Strobe)
            modeName = "Special1: Strobe";
        else if (P.activeMode == ConfigMode::Special2_EnergyBurst)
            modeName = "Special2: EnergyBurst";
        else if (P.activeMode == ConfigMode::Special3_Emergency)
            modeName = "Special3: Emergency";

        Serial.printf("Mode          : %s\n", modeName);

        // Show effect name (for Default mode) or state (for special modes)
        if (P.activeMode == ConfigMode::Default)
        {
            Serial.printf("Effect        : %s\n", fx.activeName());
        }
        else if (P.activeMode == ConfigMode::Special2_EnergyBurst)
        {
            const char *stateName = "Inactive";
            if (P.energyBurstState == EnergyBurstState::BuildingUp)
                stateName = "BuildingUp";
            else if (P.energyBurstState == EnergyBurstState::Exploding)
                stateName = "EXPLODING";
            Serial.printf("State         : %s\n", stateName);
        }

        Serial.printf("Main Color    : H=%3u S=%3u\n", P.mainHue(), P.mainSat());
        Serial.printf("Secondary     : %s\n", P.secondaryEnabled() ? "ON" : "OFF");
        Serial.printf("Sec Color     : H=%3u S=%3u\n", P.secondaryHue(), P.secondarySat());
        Serial.printf("Intensity     : %3u\n", P.intensity());
        Serial.printf("Speed         : %3u\n", P.speed());
        Serial.printf("Brightness    : %3u\n", P.brightness);
        Serial.println("-------------------------------");
    }

private:
    bool dirty = false;
    uint32_t lastPrint = 0;
};
