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
#include "SphereEffect.h"
#include "RainEffect.h"

// ============ LED Setup ============

#define MAIN_LEDS_COUNT 2
#define DETAIL_LEDS_COUNT 240
#define LED_STRING_SPACING_CM 3.0f
#define DISC_LED_STRING_COUNT 8
#define DISC_RADIUS_CM 5.0f

// ============ Effect BPM Range ============
#define MIN_BPM 50.0f
#define MAX_BPM 180.0f

// ============ Helper Functions ============

// Linearize brightness for FastLED
// FastLED uses gamma ≈ 2.2 internally, so we need to apply the inverse (raise to power 2.2)
// to get linear perceived brightness across the full 0-255 range
inline uint8_t linearizeBrightness(uint8_t input)
{
    // Normalize input to 0.0-1.0
    float normalized = input / 255.0f;

    // Apply gamma correction: output = input^2.2
    float corrected = pow(normalized, 2.2f);

    // Scale back to 0-255
    return (uint8_t)(corrected * 255.0f + 0.5f);
}

CRGB mainLeds[MAIN_LEDS_COUNT];
CRGB detailLeds[DETAIL_LEDS_COUNT];

LedEngine ledEngine(mainLeds, MAIN_LEDS_COUNT, detailLeds, DETAIL_LEDS_COUNT);

SerialHUD hud;

// Power safety
static const uint16_t BOOT_MAX_MA = 400; // Safe limit during boot (laptop USB)
static const uint16_t MAX_MA = 1400;     // Normal operating limit

// ============ Spatial Map ============
SpatialMap spatial(DETAIL_LEDS_COUNT, DISC_LED_STRING_COUNT, DISC_RADIUS_CM, LED_STRING_SPACING_CM, true);

// ============ Encoders ============
static const uint8_t DET = 4, DB = 10;

// Encoder rotation sensitivity settings
// Parameter = number of full rotations needed to go from 0 to 255
// Lower values = more sensitive (less rotation needed)
// Higher values = less sensitive (more rotation needed)
// Special: -1 = exactly 1 unit per click (no scaling)
//
// EC11 encoders have 20 detents per full rotation
// Examples:
//   1.0 rotations = 12.75 units per click (one full turn for 0→255)
//   0.5 rotations = 25.5 units per click (half turn for 0→255)
//   2.0 rotations = 6.375 units per click (two full turns for 0→255)
//   -1  rotations = 1 unit per click (direct, no scaling)
//
// enc1: Main Hue        - 2.0 rotations
// enc2: Main Saturation - 2.0 rotations
// enc3: Secondary Hue   - 20.0 rotations
// enc4: Intensity       - 1.0 rotations
// enc5: Effect/Speed    - 1.0 rotations

Encoder enc1(21, 22, 32, true, DET, DB, 2.0f); // Main Hue
Encoder enc2(16, 17, 33, true, DET, DB, 2.0f); // Main Sat
Encoder enc3(13, 14, 4, true, DET, DB, -1.0f); // Secondary Hue
Encoder enc4(18, 19, 5, true, DET, DB, 1.0f);  // Intensity
Encoder enc5(23, 25, 15, true, DET, DB, 1.0f); // Effect/Speed

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
SphereEffect sphereFx;
RainEffect rainFx;

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

        // Apply current brightness slider value before switching to normal power limit
        FastLED.setBrightness(linearizeBrightness(P.brightness));
        FastLED.show();

        // Switch to normal operating power limit
        ledEngine.setPowerLimit(5, MAX_MA);
        Serial.printf("Boot complete - switched to %dmA power limit with brightness=%d\n", MAX_MA, P.brightness);

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
    // Use safe boot power limit (400mA for laptop USB)
    ledEngine.setPowerLimit(5, BOOT_MAX_MA);
    Serial.printf("Boot mode - using %dmA power limit\n", BOOT_MAX_MA);

    // Clear all LEDs immediately
    fill_solid(mainLeds, MAIN_LEDS_COUNT, CRGB::Black);
    fill_solid(detailLeds, DETAIL_LEDS_COUNT, CRGB::Black);
    FastLED.show();

    spatial.begin();

    // Initialize config system
    configMgr.begin();

    // Configure EnergyBurst effect secondary brightness range
    // secondaryBrightnessMin (26 ≈ 10% brightness) - brightness at intensity=0
    // secondaryBrightnessMax (51 ≈ 20% brightness) - brightness at intensity=255
    // Values scale linearly with intensity parameter
    energyBurstFx.setSecondaryBrightnessRange(26, 51);

    // Configure EnergyBurst explosion height threshold
    // 0.0 = bottom of LEDs, 1.0 = top of LEDs
    // At 0.2, explosion only cancels if intensity is very low (spinning point below 20% height)
    energyBurstFx.setExplosionHeightThreshold(0.2f); // Point P to default config initially
    P.activeConfig = &configMgr.getConfig(ConfigMode::Default);
    P.activeMode = ConfigMode::Default;
    P.effectID = configMgr.getBootEffectID();

    // Register effects
    fx.add(&waveFx, "Wave");
    fx.add(&helixFx, "Helix");
    fx.add(&sphereFx, "Sphere");
    fx.add(&rainFx, "Rain");

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
                // Check height threshold (intensity maps to height ratio: 0-255 → 0.0-1.0)
                float heightRatio = P.activeConfig->intensity / 255.0f;
                float threshold = energyBurstFx.getExplosionHeightThreshold();

                if (heightRatio >= threshold)
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
                    // Cancel effect - height too low
                    configMgr.setMode(ConfigMode::Default);
                    P.activeConfig = &configMgr.getActiveConfig();
                    P.activeMode = ConfigMode::Default;
                    P.energyBurstState = EnergyBurstState::Inactive;
                    energyBurstFx.reset();
                    // Reset intensity to zero
                    configMgr.getConfig(ConfigMode::Special2_EnergyBurst).intensity = 0;
                    Serial.println("→ Default mode (Energy cancelled - height below threshold)");
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
                // Wrap effect ID (handle both positive and negative wrapping)
                // Convert to signed to handle negative deltas correctly
                int16_t effectCount = (int16_t)fx.count();
                int16_t signedEffectID = (int16_t)P.effectID;

                // Wrap around: ensure result is always in range [0, effectCount)
                signedEffectID = ((signedEffectID % effectCount) + effectCount) % effectCount;
                P.effectID = (uint8_t)signedEffectID;

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

    // Apply linearized brightness to compensate for FastLED's non-linear dimming
    FastLED.setBrightness(linearizeBrightness(P.brightness));

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
