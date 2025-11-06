#pragma once
#include <stdint.h>
#include "EffectConfig.h"

struct LightingParams
{
    // Active configuration (points to one of ConfigManager's configs)
    EffectConfig *activeConfig = nullptr;

    // Current mode
    ConfigMode activeMode = ConfigMode::Default;

    // Global settings (not per-config)
    uint8_t brightness = 150; // FastLED global brightness
    uint8_t effectID = 0;     // Current effect in Default mode

    // Special effect states
    bool strobeActive = false;                                      // Special1: Strobe is running
    EnergyBurstState energyBurstState = EnergyBurstState::Inactive; // Special2 state
    bool emergencyActive = false;                                   // Special3: Emergency lights running

    // Explosion timer for Special2
    uint32_t explosionStartTime = 0;

    // Legacy accessors for backward compatibility (delegate to activeConfig)
    uint8_t &mainHue() { return activeConfig->mainHue; }
    uint8_t &mainSat() { return activeConfig->mainSat; }
    uint8_t &secondaryHue() { return activeConfig->secondaryHue; }
    uint8_t &secondarySat() { return activeConfig->secondarySat; }
    uint8_t &speed() { return activeConfig->speed; }
    uint8_t &intensity() { return activeConfig->intensity; }
    bool &secondaryEnabled() { return activeConfig->secondaryEnabled; }

    // Const accessors
    uint8_t mainHue() const { return activeConfig->mainHue; }
    uint8_t mainSat() const { return activeConfig->mainSat; }
    uint8_t secondaryHue() const { return activeConfig->secondaryHue; }
    uint8_t secondarySat() const { return activeConfig->secondarySat; }
    uint8_t speed() const { return activeConfig->speed; }
    uint8_t intensity() const { return activeConfig->intensity; }
    bool secondaryEnabled() const { return activeConfig->secondaryEnabled; }
};
