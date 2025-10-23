#pragma once
#include <Arduino.h>
#include <FastLED.h>

struct LedParams
{
    uint8_t brightness = 150; // 0..255
    uint8_t hue = 0;          // 0..255
    uint8_t sat = 255;        // 0..255
    uint8_t speed = 32;       // 0..255 (effect step rate)
    uint8_t effect = 0;       // which effect
};

class LedEngine
{
public:
    // Two-strip constructor
    LedEngine(uint8_t pin1, int num1, CRGB *buf1,
              uint8_t pin2, int num2, CRGB *buf2)
        : pin1(pin1), n1(num1), leds1(buf1),
          pin2(pin2), n2(num2), leds2(buf2) {}

    void begin();
    void setParams(const LedParams &p)
    {
        params = p;
        FastLED.setBrightness(params.brightness);
    }
    LedParams &mutableParams() { return params; } // direct tweak
    void nextEffect(uint8_t count = 1);
    void prevEffect(uint8_t count = 1);

    void setPowerLimit(uint8_t volts, uint16_t milliamps);

    // Call every loop; keeps boot animation for the first few seconds.
    void update(uint32_t nowMs);

    // Optional — call if you want to restart the boot animation
    void restartBoot(uint32_t nowMs);

private:
    uint8_t pin1, pin2;
    int n1, n2;
    CRGB *leds1;
    CRGB *leds2;

    LedParams params;

    // Boot animation
    uint32_t bootStart = 0;
    bool bootActive = true;
    uint8_t bootHue = 0;
    uint32_t lastBootStep = 0;

    // Effect state
    uint32_t lastFxStep = 0;
    uint8_t fxPhase = 0;

    // Power management
    uint8_t limitVolts = 5;
    uint16_t limitMilliamps = 500;

    // Render helpers
    void renderBoot(uint32_t nowMs);
    void renderEffect(uint32_t nowMs);
    void fxSolid();
    void fxRainbow(uint32_t nowMs);
    void fxTheater(uint32_t nowMs);
    void clearAll();
};
