#include <Arduino.h>
#include <FastLED.h>
#include <Encoder.h>
#include "LightingParams.h"
#include "SpatialMap.h"
#include "EffectManager.h"

#include "InputManager.h"
#include "InputMapper.h"
#include "Pot.h"
#include "LedEngine.h"
#include "SerialHUD.h"

#include "SolidColorEffect.h"
#include "SpatialWaveEffect.h"
#include "DoubleHelixEffect.h"

// ============ LED Setup ============

#define MAIN_COUNT 2
#define DETAIL_COUNT 240

CRGB mainLeds[MAIN_COUNT];
CRGB detailLeds[DETAIL_COUNT];

LedEngine ledEngine(mainLeds, MAIN_COUNT, detailLeds, DETAIL_COUNT);

SerialHUD hud;

// Power safety
static const uint16_t MAX_MA = 400;

// ============ Spatial Map ============
SpatialMap spatial(DETAIL_COUNT, 10, 10.0f, 3.0f, true);

// ============ Encoders ============
static const uint8_t DET = 4, DB = 10;

Encoder enc1(21, 22, 0, true, DET, DB);
Encoder enc2(16, 17, 32, true, DET, DB);
Encoder enc3(13, 14, 4, true, DET, DB);
Encoder enc4(18, 19, 5, true, DET, DB);
Encoder enc5(23, 25, 15, true, DET, DB);

Encoder *encs[] = {&enc1, &enc2, &enc3, &enc4, &enc5};

// ============ Pot ============
#define POT_PIN 36
Pot pot(POT_PIN);

// ============ Input system ============
InputManager input(encs, 5, &pot);
InputMapper mapper;

// ============ Effects ============
EffectManager fx;
SolidColorEffect solidFx;
SpatialWaveEffect waveFx;
DoubleHelixEffect helixFx;

// Lighting state
LightingParams P;

// ============ Boot Animation ============
uint8_t bootHue = 0;
uint32_t bootStart;
bool bootActive = true;

void bootAnimation(uint32_t now)
{
    if (!bootActive)
        return;
    if (now - bootStart > 3000)
    {
        bootActive = false;
        ledEngine.clearAll();
        FastLED.show();
        return;
    }
    fill_rainbow(mainLeds, MAIN_COUNT, bootHue, 8);
    fill_rainbow(detailLeds, DETAIL_COUNT, bootHue + 64, 4);
    bootHue++;
    FastLED.show();
}

// ============ Strobe Overlay ============
static inline uint16_t strobePeriodMs(uint8_t s)
{
    // 255 -> ~30ms, 0 -> ~1000ms
    return (uint16_t)(1000 - (s * 970) / 255) + 30;
}

void applyStrobe(uint32_t now)
{
    if (!P.strobe)
        return;

    const uint16_t period = strobePeriodMs(P.strobeSpeed);
    const bool on = (now % period) < (period / 2);

    if (on)
    {
        // Option C: main = white flash, detail off
        fill_solid(mainLeds, MAIN_COUNT, CRGB::White);
        fill_solid(detailLeds, DETAIL_COUNT, CRGB::Black);
    }
    else
    {
        // Off phase: keep detail off, main off too
        fill_solid(mainLeds, MAIN_COUNT, CRGB::Black);
        fill_solid(detailLeds, DETAIL_COUNT, CRGB::Black);
    }
    // DO NOT call FastLED.show() here; main loop shows once after overlays.
}

// ================= MAIN =================
void setup()
{
    Serial.begin(115200);

    input.begin();
    pot.begin();

    ledEngine.begin();
    ledEngine.setPowerLimit(5, MAX_MA);

    spatial.begin();

    fx.add(&solidFx, "Solid");
    fx.add(&waveFx, "Wave");
    fx.add(&helixFx, "Helix");

    hud.begin();

    bootStart = millis();
}

void loop()
{
    uint32_t now = millis();

    // Boot
    if (bootActive)
    {
        bootAnimation(now);
        return;
    }

    // Read input
    InputEvent ev;
    if (input.poll(ev))
    {
        if (mapper.apply(ev, P))
        {
            // Wrap to #effects
            P.effectID %= fx.count();
            if (fx.setEffect(P.effectID))
            {
                hud.markDirty();
            }
        }
    }

    FastLED.setBrightness(P.brightness);

    // Render base effect
    fx.setEffect(P.effectID);
    fx.render(P, spatial,
              mainLeds, MAIN_COUNT,
              detailLeds, DETAIL_COUNT,
              now);

    hud.update(P, fx, now);

    // Apply overlays (strobe)
    applyStrobe(now);

    FastLED.show();
}
