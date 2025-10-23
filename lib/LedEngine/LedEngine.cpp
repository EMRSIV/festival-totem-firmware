#include "LedEngine.h"

void LedEngine::begin()
{
    FastLED.addLeds<NEOPIXEL, 26>(leds1, n1); // PIN doesn’t matter here, FastLED bound at compile; keep addLeds in main for dynamic pins.
    // NOTE: addLeds must be called in main with correct template pins.
    // We keep begin() minimal and rely on main to add both strips.
    FastLED.setBrightness(params.brightness);
}

void LedEngine::setPowerLimit(uint8_t volts, uint16_t milliamps)
{
    limitVolts = volts;
    limitMilliamps = milliamps;
    FastLED.setMaxPowerInVoltsAndMilliamps(limitVolts, limitMilliamps);
}

void LedEngine::restartBoot(uint32_t nowMs)
{
    bootStart = nowMs;
    bootActive = true;
    bootHue = 0;
    lastBootStep = nowMs;
}

void LedEngine::nextEffect(uint8_t count)
{
    params.effect = (params.effect + count) % 3; // we have 3 demo effects: 0..2
}
void LedEngine::prevEffect(uint8_t count)
{
    params.effect = (uint8_t)((params.effect + 3 - (count % 3)) % 3);
}

void LedEngine::update(uint32_t nowMs)
{
    if (bootActive)
    {
        renderBoot(nowMs);
        return;
    }
    renderEffect(nowMs);
}

void LedEngine::renderBoot(uint32_t nowMs)
{
    if (nowMs - bootStart > 3000)
    {
        bootActive = false;
        clearAll();
        FastLED.show();
        return;
    }
    if (nowMs - lastBootStep > 20)
    {
        lastBootStep = nowMs;
        fill_rainbow(leds1, n1, bootHue, 3);
        fill_rainbow(leds2, n2, bootHue + 128, 3);
        bootHue++;
        FastLED.show();
    }
}

void LedEngine::renderEffect(uint32_t nowMs)
{
    switch (params.effect)
    {
    case 0:
        fxSolid();
        break;
    case 1:
        fxRainbow(nowMs);
        break;
    case 2:
        fxTheater(nowMs);
        break;
    default:
        fxSolid();
        break;
    }
}

// ----- Effects -----
void LedEngine::fxSolid()
{
    CHSV color(params.hue, params.sat, 255);
    fill_solid(leds1, n1, color);
    fill_solid(leds2, n2, color);
    FastLED.show();
}

void LedEngine::fxRainbow(uint32_t nowMs)
{
    const uint8_t stepDelay = 4 + (255 - params.speed) / 8; // faster when speed is higher
    if (nowMs - lastFxStep < stepDelay)
        return;
    lastFxStep = nowMs;

    static uint8_t baseHue = 0;
    baseHue += (params.speed > 0 ? 1 : 0);
    fill_rainbow(leds1, n1, baseHue, 3);
    fill_rainbow(leds2, n2, baseHue + 64, 3);
    FastLED.show();
}

void LedEngine::fxTheater(uint32_t nowMs)
{
    const uint8_t stepDelay = 20 + (255 - params.speed) / 4;
    if (nowMs - lastFxStep < stepDelay)
        return;
    lastFxStep = nowMs;

    fxPhase = (fxPhase + 1) % 3;
    CHSV onColor(params.hue, params.sat, 255);

    for (int i = 0; i < n1; ++i)
        leds1[i] = ((i % 3) == fxPhase) ? CRGB(onColor) : CRGB::Black;

    for (int i = 0; i < n2; ++i)
        leds2[i] = ((i % 3) == fxPhase) ? CRGB(onColor) : CRGB::Black;

    FastLED.show();
}

void LedEngine::clearAll()
{
    fill_solid(leds1, n1, CRGB::Black);
    fill_solid(leds2, n2, CRGB::Black);
}
