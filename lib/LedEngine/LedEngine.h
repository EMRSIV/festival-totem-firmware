#pragma once
#include <FastLED.h>

#define LED_PIN_MAIN 27
#define LED_PIN_DETAIL 26
#define LED_TYPE_MAIN BRG
#define LED_TYPE_DETAIL GRB

class LedEngine
{
public:
    LedEngine(CRGB *mainBuf, uint16_t mainCount,
              CRGB *detailBuf, uint16_t detailCount);

    void begin();
    void setPowerLimit(uint8_t volts, uint16_t milliamps);

    CRGB *mainLeds;
    CRGB *detailLeds;
    uint16_t nMain, nDetail;

    void clearAll();
};
