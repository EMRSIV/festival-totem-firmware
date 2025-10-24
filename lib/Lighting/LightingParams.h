#pragma once
#include <stdint.h>

struct LightingParams
{
    // Colors
    uint8_t mainHue = 0;
    uint8_t mainSat = 255;
    uint8_t secondaryHue = 128;
    uint8_t secondarySat = 255;

    // Behavior
    uint8_t speed = 100;      // animation speed
    uint8_t intensity = 255;  // effect strength
    uint8_t brightness = 150; // FastLED brightness
    uint8_t effectID = 0;     // current effect

    // Toggles / modes
    bool secondaryEnabled = true;
    bool strobe = false;
    uint8_t strobeSpeed = 128; // 0..255, higher = faster

    bool special1 = false;
    bool special2 = false;
};
