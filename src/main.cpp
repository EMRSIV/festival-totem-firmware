#include <Arduino.h>
#include <FastLED.h>
#include <Encoder.h>
#include "Pot.h"
#include "LedEngine.h"

// ===================== Pins & lengths =====================
#define LED_PIN1 26
#define LED_PIN2 27
#define NUM_LEDS_1 2
#define NUM_LEDS_2 300

#define POT_PIN 36 // ADC1_CH0

// ===================== Encoders =====================
static const uint8_t DETENT_TRANSITIONS = 4;
static const uint16_t DEBOUNCE_MS = 10;

Encoder enc1(13, 14, 0, true, DETENT_TRANSITIONS, DEBOUNCE_MS);  // HUE
Encoder enc2(16, 17, 32, true, DETENT_TRANSITIONS, DEBOUNCE_MS); // BRIGHTNESS
Encoder enc3(21, 22, 4, true, DETENT_TRANSITIONS, DEBOUNCE_MS);  // SPEED
Encoder enc4(18, 19, 5, true, DETENT_TRANSITIONS, DEBOUNCE_MS);  // EFFECT SELECT
Encoder enc5(23, 25, 15, true, DETENT_TRANSITIONS, DEBOUNCE_MS); // SATURATION

Encoder *ENCS[] = {&enc1, &enc2, &enc3, &enc4, &enc5};
int32_t lastDet[5] = {0, 0, 0, 0, 0};

// ===================== LEDs =====================
CRGB leds1[NUM_LEDS_1];
CRGB leds2[NUM_LEDS_2];
LedEngine engine(LED_PIN1, NUM_LEDS_1, leds1,
                 LED_PIN2, NUM_LEDS_2, leds2);

static const uint16_t MAX_MA = 2000; // PSU power supply limit (keep a 300mA margin)

// Parameters we’ll control generically
LedParams P;

// ===================== Pot =====================
Pot pot(POT_PIN, 12, ADC_11db, 0.12f);

// ===================== Helpers =====================
template <typename T>
static inline T clampv(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Map detent delta from an encoder into a parameter with step size
static void applyDetentDeltaToParam(int encIdx, int delta)
{
    switch (encIdx)
    {
    case 0:                                   // HUE
        P.hue = (uint8_t)(P.hue + delta * 3); // 3 per detent
        break;
    case 1: // BRIGHTNESS
        P.brightness = clampv<int>(P.brightness + delta * 5, 0, 255);
        FastLED.setBrightness(P.brightness);
        break;
    case 2: // SPEED
        P.speed = clampv<int>(P.speed + delta * 4, 0, 255);
        break;
    case 3: // EFFECT SELECT
        if (delta > 0)
            engine.nextEffect(delta);
        else if (delta < 0)
            engine.prevEffect(-delta);
        break;
    case 4: // SATURATION
        P.sat = clampv<int>(P.sat + delta * 5, 0, 255);
        break;
    }
}

// Button actions (press = next effect, long press could be added later)
static void attachButtonCallbacks()
{
    // Enc4 button: next effect
    ENCS[3]->onPress([]
                     {
                         // single press also cycles effect by 1
                         // (encoder rotation also changes effect via applyDetentDeltaToParam)
                         // Using nextEffect keeps UX consistent.
                     });
    // Enc1 press: reset hue to 0
    ENCS[0]->onPress([] { /* placeholder if you want special actions */ });
    // Add more as you like…
}

// ===================== Setup =====================
void setup()
{
    Serial.begin(115200);
    Serial.println("\nLED Controller Boot");

    // Encoders
    for (uint8_t i = 0; i < 5; ++i)
    {
        ENCS[i]->begin();
        lastDet[i] = ENCS[i]->getDetentCount();
    }
    attachButtonCallbacks();

    // Pot
    pot.begin();

    // FastLED setup (must be here, not inside LedEngine.cpp because of template pins)
    FastLED.addLeds<NEOPIXEL, LED_PIN1>(leds1, NUM_LEDS_1);
    FastLED.addLeds<NEOPIXEL, LED_PIN2>(leds2, NUM_LEDS_2);
    FastLED.setBrightness(P.brightness);

    engine.setPowerLimit(5, MAX_MA);

    // Boot animation
    engine.setParams(P);
    engine.restartBoot(millis());
}

// ===================== Loop =====================
void loop()
{
    // Update inputs
    for (uint8_t i = 0; i < 5; ++i)
        ENCS[i]->update();
    pot.update();

    // Handle encoder detent deltas
    for (uint8_t i = 0; i < 5; ++i)
    {
        int32_t d = ENCS[i]->getDetentCount();
        int delta = (int)(d - lastDet[i]);
        if (delta != 0)
        {
            applyDetentDeltaToParam(i, delta);
            lastDet[i] = d;
            Serial.print("[ENC");
            Serial.print(i + 1);
            Serial.print("] delta=");
            Serial.print(delta);
            Serial.print("  -> (H:");
            Serial.print(P.hue);
            Serial.print(", S:");
            Serial.print(P.sat);
            Serial.print(", B:");
            Serial.print(P.brightness);
            Serial.print(", V:");
            Serial.print(P.speed);
            Serial.println(")");
        }
    }

    // Map pot (0..255) to brightness (or whatever you prefer)
    uint8_t pot8 = pot.value8();
    // Example: make pot master brightness (overriding encoder)
    P.brightness = pot8;
    FastLED.setBrightness(P.brightness);

    // Push params into engine & render
    engine.setParams(P);
    engine.update(millis());

    delay(2);
}
