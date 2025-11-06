#include <Arduino.h>
#include <FastLED.h>
#include <Encoder.h>
#include "LightingParams.h"
#include "SpatialMap.h"
#include "EffectManager.h"
#include "ConfigManager.h"

#include "InputManager.h"
#include "InputMapper.h"
#include "Pot.h"
#include "LedEngine.h"
#include "SerialHUD.h"

#include "SpatialWaveEffect.h"
#include "DoubleHelixEffect.h"
#include "EnergyBurstEffect.h"
#include "EmergencyEffect.h"

// ============ LED Setup ============

#define MAIN_LEDS_COUNT 2
#define DETAIL_LEDS_COUNT 240
#define LED_STRING_SPACING_CM 3.0f
#define DISC_LED_STRING_COUNT 8
#define DISC_RADIUS_CM 5.0f

CRGB mainLeds[MAIN_LEDS_COUNT];
CRGB detailLeds[DETAIL_LEDS_COUNT];

LedEngine ledEngine(mainLeds, MAIN_LEDS_COUNT, detailLeds, DETAIL_LEDS_COUNT);

SerialHUD hud;

// Power safety
static const uint16_t MAX_MA = 400;

// ============ Spatial Map ============
SpatialMap spatial(DETAIL_LEDS_COUNT, DISC_LED_STRING_COUNT, DISC_RADIUS_CM, LED_STRING_SPACING_CM, true);

// ============ Encoders ============
static const uint8_t DET = 4, DB = 10;

Encoder enc1(21, 22, 0, true, DET, DB);
Encoder enc2(16, 17, 33, true, DET, DB);
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

// ============ Config System ============
ConfigManager configMgr;

// ============ Effects ============
EffectManager fx;
SpatialWaveEffect waveFx;
DoubleHelixEffect helixFx;
EnergyBurstEffect energyBurstFx;
EmergencyEffect emergencyFx;

// Lighting state
LightingParams P;

// ============ Boot Animation ============
#define BOOT_SEQUENCE_LENGTH 500
uint32_t bootStart;
bool bootActive = true;

void bootAnimation(uint32_t now)
{
    if (!bootActive)
        return;
    if (now - bootStart > BOOT_SEQUENCE_LENGTH)
    {
        bootActive = false;
        ledEngine.clearAll();
        FastLED.show();
        return;
    }

    // Simple strobe: main color on main LEDs, secondary on detail LEDs, alternating
    uint32_t elapsed = now - bootStart;
    uint16_t period = BOOT_SEQUENCE_LENGTH / 20;
    bool showMain = ((elapsed / period) % 2) == 0;

    // Use default config colors
    EffectConfig &defaultConfig = configMgr.getConfig(ConfigMode::Default);
    CRGB mainColor = CHSV(defaultConfig.mainHue, defaultConfig.mainSat, 255);
    CRGB secondaryColor = CHSV(defaultConfig.secondaryHue, defaultConfig.secondarySat, 255);

    if (showMain)
    {
        // Main color phase: main LEDs lit, detail LEDs off
        fill_solid(mainLeds, MAIN_LEDS_COUNT, mainColor);
        fill_solid(detailLeds, DETAIL_LEDS_COUNT, CRGB::Black);
    }
    else
    {
        // Secondary color phase: detail LEDs lit, main LEDs off
        fill_solid(mainLeds, MAIN_LEDS_COUNT, CRGB::Black);
        fill_solid(detailLeds, DETAIL_LEDS_COUNT, secondaryColor);
    }

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
    if (!P.strobeActive)
        return;

    // Get strobe color from Special1 config
    EffectConfig &strobeConfig = configMgr.getConfig(ConfigMode::Special1_Strobe);
    uint16_t period = strobePeriodMs(strobeConfig.speed);
    const bool on = (now % period) < (period / 2);

    if (on)
    {
        // Adjustable color strobe
        CRGB strobeColor = CHSV(strobeConfig.mainHue, strobeConfig.mainSat, 255);
        fill_solid(mainLeds, MAIN_LEDS_COUNT, strobeColor);
        fill_solid(detailLeds, DETAIL_LEDS_COUNT, CRGB::Black);
    }
    else
    {
        // Off phase
        fill_solid(mainLeds, MAIN_LEDS_COUNT, CRGB::Black);
        fill_solid(detailLeds, DETAIL_LEDS_COUNT, CRGB::Black);
    }
}

// ============ Save Feedback ============
void showSaveFeedback()
{
    // 0.5s green strobe on all LEDs
    for (int i = 0; i < 5; i++)
    {
        fill_solid(mainLeds, MAIN_LEDS_COUNT, CRGB::Green);
        fill_solid(detailLeds, DETAIL_LEDS_COUNT, CRGB::Green);
        FastLED.show();
        delay(50);

        fill_solid(mainLeds, MAIN_LEDS_COUNT, CRGB::Black);
        fill_solid(detailLeds, DETAIL_LEDS_COUNT, CRGB::Black);
        FastLED.show();
        delay(50);
    }
    Serial.println("✅ Configs saved!");
}

// ================= MAIN =================
void setup()
{
    Serial.begin(115200);
    delay(100);
    Serial.println("\n=== Festival Totem Firmware ===");

    input.begin();
    pot.begin();

    ledEngine.begin();
    ledEngine.setPowerLimit(5, MAX_MA);

    // Clear all LEDs immediately
    fill_solid(mainLeds, MAIN_LEDS_COUNT, CRGB::Black);
    fill_solid(detailLeds, DETAIL_LEDS_COUNT, CRGB::Black);
    FastLED.show();

    spatial.begin();

    // Initialize config system
    configMgr.begin();

    // Point P to default config initially
    P.activeConfig = &configMgr.getConfig(ConfigMode::Default);
    P.activeMode = ConfigMode::Default;
    P.effectID = configMgr.getBootEffectID();

    // Register effects
    fx.add(&waveFx, "Wave");
    fx.add(&helixFx, "Helix");

    hud.begin();

    bootStart = millis();

    Serial.println("Startup complete!");
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

    // Read input and handle mode switching
    InputEvent ev;
    if (input.poll(ev))
    {
        // Handle mode switching events
        switch (ev.action)
        {
        case InputAction::EnterSpecial1: // Enc5 pressed
            // Cancel EnergyBurst if active
            if (P.energyBurstState != EnergyBurstState::Inactive)
            {
                P.energyBurstState = EnergyBurstState::Inactive;
                energyBurstFx.reset();
                // Reset intensity to zero
                configMgr.getConfig(ConfigMode::Special2_EnergyBurst).intensity = 0;
            }
            configMgr.setMode(ConfigMode::Special1_Strobe);
            P.activeConfig = &configMgr.getActiveConfig();
            P.activeMode = ConfigMode::Special1_Strobe;
            P.strobeActive = true;
            Serial.println("→ Special1: Strobe ON");
            hud.markDirty();
            break;

        case InputAction::ExitSpecial1: // Enc5 released
            configMgr.setMode(ConfigMode::Default);
            P.activeConfig = &configMgr.getActiveConfig();
            P.activeMode = ConfigMode::Default;
            P.strobeActive = false;
            Serial.println("→ Default mode");
            hud.markDirty();
            break;

        case InputAction::EnterSpecial2: // Enc4 pressed
        {
            EnergyBurstState currentState = P.energyBurstState;

            if (currentState == EnergyBurstState::Inactive)
            {
                // Start energy buildup
                configMgr.setMode(ConfigMode::Special2_EnergyBurst);
                P.activeConfig = &configMgr.getActiveConfig();
                P.activeMode = ConfigMode::Special2_EnergyBurst;
                P.energyBurstState = EnergyBurstState::BuildingUp;
                energyBurstFx.setState(EnergyBurstState::BuildingUp);
                Serial.println("→ Special2: Energy BuildUp");
                hud.markDirty();
            }
            else if (currentState == EnergyBurstState::BuildingUp)
            {
                // Check intensity threshold
                if (P.activeConfig->intensity > 242) // 95% of 255
                {
                    // Trigger explosion
                    P.energyBurstState = EnergyBurstState::Exploding;
                    energyBurstFx.setState(EnergyBurstState::Exploding);
                    P.explosionStartTime = now;
                    Serial.println("→ Special2: EXPLOSION!");
                    hud.markDirty();
                }
                else
                {
                    // Cancel effect
                    configMgr.setMode(ConfigMode::Default);
                    P.activeConfig = &configMgr.getActiveConfig();
                    P.activeMode = ConfigMode::Default;
                    P.energyBurstState = EnergyBurstState::Inactive;
                    energyBurstFx.reset();
                    // Reset intensity to zero
                    configMgr.getConfig(ConfigMode::Special2_EnergyBurst).intensity = 0;
                    Serial.println("→ Default mode (Energy cancelled)");
                    hud.markDirty();
                }
            }
            break;
        }

        case InputAction::EnterSpecial3: // Enc3 pressed
            // Cancel EnergyBurst if active
            if (P.energyBurstState != EnergyBurstState::Inactive)
            {
                P.energyBurstState = EnergyBurstState::Inactive;
                energyBurstFx.reset();
                // Reset intensity to zero
                configMgr.getConfig(ConfigMode::Special2_EnergyBurst).intensity = 0;
            }
            configMgr.setMode(ConfigMode::Special3_Emergency);
            P.activeConfig = &configMgr.getActiveConfig();
            P.activeMode = ConfigMode::Special3_Emergency;
            P.emergencyActive = true;
            Serial.println("→ Special3: Emergency Lights ON");
            hud.markDirty();
            break;

        case InputAction::ExitSpecial3: // Enc3 released
            configMgr.setMode(ConfigMode::Default);
            P.activeConfig = &configMgr.getActiveConfig();
            P.activeMode = ConfigMode::Default;
            P.emergencyActive = false;
            Serial.println("→ Default mode");
            hud.markDirty();
            break;

        case InputAction::SaveConfigs: // Enc4 + Enc5 held 5s
            configMgr.setBootEffectID(P.effectID);
            configMgr.save();
            showSaveFeedback();
            break;

        default:
            // Let mapper handle all other actions
            if (mapper.apply(ev, P))
            {
                // Wrap effect ID
                P.effectID %= fx.count();
                if (fx.setEffect(P.effectID))
                {
                    hud.markDirty();
                }
                // Mark dirty for any parameter changes
                hud.markDirty();
            }
            break;
        }
    }

    // Check if explosion finished (auto-exit after 2s)
    if (P.energyBurstState == EnergyBurstState::Exploding)
    {
        if (now - P.explosionStartTime >= 2000)
        {
            configMgr.setMode(ConfigMode::Default);
            P.activeConfig = &configMgr.getActiveConfig();
            P.activeMode = ConfigMode::Default;
            P.energyBurstState = EnergyBurstState::Inactive;
            energyBurstFx.reset();
            // Reset intensity to zero
            configMgr.getConfig(ConfigMode::Special2_EnergyBurst).intensity = 0;
            Serial.println("→ Default mode (Explosion complete)");
            hud.markDirty();
        }
    }

    FastLED.setBrightness(P.brightness);

    // Render based on active mode
    if (P.activeMode == ConfigMode::Special2_EnergyBurst &&
        P.energyBurstState != EnergyBurstState::Inactive)
    {
        // Render energy burst effect
        energyBurstFx.render(P, spatial, mainLeds, MAIN_LEDS_COUNT, detailLeds, DETAIL_LEDS_COUNT, now);
    }
    else if (P.activeMode == ConfigMode::Special3_Emergency && P.emergencyActive)
    {
        // Render emergency effect
        emergencyFx.render(P, spatial, mainLeds, MAIN_LEDS_COUNT, detailLeds, DETAIL_LEDS_COUNT, now);
    }
    else if (P.activeMode == ConfigMode::Default)
    {
        // Render normal effects
        fx.setEffect(P.effectID);
        fx.render(P, spatial, mainLeds, MAIN_LEDS_COUNT, detailLeds, DETAIL_LEDS_COUNT, now);
    }

    hud.update(P, fx, now);

    // Apply strobe overlay (Special1) if active
    if (P.strobeActive)
    {
        applyStrobe(now);
    }

    FastLED.show();
}
