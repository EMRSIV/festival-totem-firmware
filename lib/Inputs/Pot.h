#pragma once
#include <Arduino.h>

class Pot
{
public:
    Pot(uint8_t pin, uint8_t resolutionBits = 12, adc_attenuation_t attn = ADC_11db,
        float alpha = 0.12f)
        : pin(pin), bits(resolutionBits), attn(attn), alpha(alpha) {}

    void begin()
    {
        analogReadResolution(bits);
        analogSetPinAttenuation(pin, attn);
        smooth = analogRead(pin);
        lastReported = -10000;
    }

    void update()
    {
        float v = analogRead(pin);
        smooth = (1.0f - alpha) * smooth + alpha * v;
    }

    uint16_t valueRaw() const { return (uint16_t)(smooth + 0.5f); }

    uint8_t value8() const
    {
        float maxV = (float)((1U << bits) - 1);
        return (uint8_t)((smooth / maxV) * 255.0f + 0.5f);
    }

    bool changed(uint16_t delta = 8)
    {
        uint16_t cur = valueRaw();
        if (abs((int)cur - (int)lastReported) >= (int)delta)
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
    float lastReported;
};
