#pragma once
#include <stdint.h>

// Per-mode effect configuration
struct EffectConfig
{
    uint8_t mainHue = 0;
    uint8_t mainSat = 255;
    uint8_t secondaryHue = 128;
    uint8_t secondarySat = 255;
    // Default speed ~50% (0-255 scale)
    uint8_t speed = 127;
    // Default intensity
    uint8_t intensity = 127;
    bool secondaryEnabled = true;
};

// System operation modes
enum class ConfigMode : uint8_t
{
    Default = 0,              // Normal effect cycling
    Special1_Strobe = 1,      // Enc5 hold: Strobe with adjustable color
    Special2_EnergyBurst = 2, // Enc4 press: Energy buildup → explosion
    Special3_Emergency = 3    // Enc3 hold: Blue/red emergency lights
};

// Energy Burst effect states
enum class EnergyBurstState : uint8_t
{
    Inactive = 0,   // Not running
    BuildingUp = 1, // Spinning point rising with intensity
    Exploding = 2   // Fast main LED alternating (2s duration)
};
