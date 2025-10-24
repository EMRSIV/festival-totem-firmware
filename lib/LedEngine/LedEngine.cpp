#include "LedEngine.h"

LedEngine::LedEngine(CRGB *mainBuf, uint16_t mc,
                     CRGB *detailBuf, uint16_t dc)
    : mainLeds(mainBuf), nMain(mc),
      detailLeds(detailBuf), nDetail(dc)
{
}

void LedEngine::begin()
{
    // Use compile-time #defines from main.cpp
    FastLED.addLeds<WS2812B, LED_PIN_MAIN, LED_TYPE_MAIN>(mainLeds, nMain);
    FastLED.addLeds<WS2812B, LED_PIN_DETAIL, LED_TYPE_DETAIL>(detailLeds, nDetail);
}

void LedEngine::setPowerLimit(uint8_t volts, uint16_t ma)
{
    FastLED.setMaxPowerInVoltsAndMilliamps(volts, ma);
}

void LedEngine::clearAll()
{
    fill_solid(mainLeds, nMain, CRGB::Black);
    fill_solid(detailLeds, nDetail, CRGB::Black);
}
