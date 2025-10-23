#pragma once
#include <Arduino.h>

class Pot
{
public:
    explicit Pot(uint8_t pin, uint8_t resolutionBits = 12, adc_attenuation_t attn = ADC_11db,
                 float alpha = 0.12f)
        : pin(pin), bits(resolutionBits), attn(attn), alpha(alpha) {}

    void begin()
    {
        analogReadResolution(bits);
        analogSetPinAttenuation(pin, attn);
        float v = analogRead(pin);
        smooth = v;
        lastReported = -1e9f;
    }

    // Call often
    void update()
    {
        float v = analogRead(pin);
        smooth = (1.0f - alpha) * smooth + alpha * v;
    }

    // 0..(2^bits-1)
    uint16_t valueRaw() const { return (uint16_t)(smooth + 0.5f); }

    // 0..255 scaled
    uint8_t value8() const
    {
        const float maxV = (float)((1u << bits) - 1);
        float x = (smooth / maxV) * 255.0f;
        if (x < 0)
            x = 0;
        if (x > 255)
            x = 255;
        return (uint8_t)(x + 0.5f);
    }

    // Print only when it changed by >= delta
    bool changed(uint16_t delta = 8)
    {
        uint16_t cur = valueRaw();
        if (lastReported < 0 || abs((int)cur - (int)lastReported) >= (int)delta)
        {
            lastReported = cur;
            return true;
        }
        return false;
    }

private:
    uint8_t pin;
    uint8_t bits;
    adc_attenuation_t attn;
    float alpha;

    float smooth = 0;
    float lastReported = -1e9f;
};
