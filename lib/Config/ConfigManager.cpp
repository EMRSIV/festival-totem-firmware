#include "ConfigManager.h"
#include <Arduino.h>

ConfigManager::ConfigManager()
    : activeMode(ConfigMode::Default),
      energyBurstState(EnergyBurstState::Inactive),
      bootEffectID(0)
{
    // Initialize default configs with sensible values
    // Default mode - uses struct defaults from EffectConfig.h
    // (speed=127 ~50%, intensity=0)

    // Special1 - Strobe (default white)
    configs[1].mainHue = 0;
    configs[1].mainSat = 0; // White
    configs[1].speed = 248;
    configs[1].intensity = 255;
    configs[1].secondaryEnabled = false;

    // Special2 - Energy Burst
    configs[2].mainHue = 2; // Blue-ish
    configs[2].mainSat = 255;
    configs[2].secondaryHue = 250; // Orange for droplets
    configs[2].secondarySat = 255;
    configs[2].speed = 127;   // 50%
    configs[2].intensity = 0; // Mid-height
    configs[2].secondaryEnabled = true;

    // Special3 - Emergency
    configs[3].speed = 127;
    configs[3].intensity = 255;
    configs[3].secondaryEnabled = false;
}

void ConfigManager::begin()
{
    prefs.begin("totem", false); // Read-write mode

    // Load only Default config from NVS (special configs are runtime-only)
    loadConfig(ConfigMode::Default);

    // Load boot effect ID
    bootEffectID = prefs.getUChar("boot_fx", 0);

    Serial.println("ConfigManager: Loaded from NVS");
}

void ConfigManager::save()
{
    // Save only Default config (special configs are runtime-only)
    saveConfig(ConfigMode::Default);

    // Save boot effect ID
    prefs.putUChar("boot_fx", bootEffectID);

    Serial.println("ConfigManager: Saved to NVS");
}

void ConfigManager::setMode(ConfigMode mode)
{
    if (activeMode != mode)
    {
        activeMode = mode;
        Serial.printf("ConfigManager: Mode switched to %d\n", (int)mode);
    }
}

EffectConfig &ConfigManager::getActiveConfig()
{
    return configs[(uint8_t)activeMode];
}

EffectConfig &ConfigManager::getConfig(ConfigMode mode)
{
    return configs[(uint8_t)mode];
}

void ConfigManager::setBootEffectID(uint8_t id)
{
    bootEffectID = id;
}

void ConfigManager::setEnergyBurstState(EnergyBurstState state)
{
    if (energyBurstState != state)
    {
        energyBurstState = state;
        Serial.printf("EnergyBurst: State changed to %d\n", (int)state);
    }
}

const char *ConfigManager::getKeyPrefix(ConfigMode mode)
{
    switch (mode)
    {
    case ConfigMode::Default:
        return "d";
    case ConfigMode::Special1_Strobe:
        return "s1";
    case ConfigMode::Special2_EnergyBurst:
        return "s2";
    case ConfigMode::Special3_Emergency:
        return "s3";
    default:
        return "d";
    }
}

void ConfigManager::loadConfig(ConfigMode mode)
{
    const char *prefix = getKeyPrefix(mode);
    EffectConfig &cfg = configs[(uint8_t)mode];

    char key[8];

    sprintf(key, "%s_mh", prefix);
    cfg.mainHue = prefs.getUChar(key, cfg.mainHue);

    sprintf(key, "%s_ms", prefix);
    cfg.mainSat = prefs.getUChar(key, cfg.mainSat);

    sprintf(key, "%s_sh", prefix);
    cfg.secondaryHue = prefs.getUChar(key, cfg.secondaryHue);

    sprintf(key, "%s_ss", prefix);
    cfg.secondarySat = prefs.getUChar(key, cfg.secondarySat);

    sprintf(key, "%s_spd", prefix);
    cfg.speed = prefs.getUChar(key, cfg.speed);

    sprintf(key, "%s_int", prefix);
    cfg.intensity = prefs.getUChar(key, cfg.intensity);

    sprintf(key, "%s_sec", prefix);
    cfg.secondaryEnabled = prefs.getBool(key, cfg.secondaryEnabled);
}

void ConfigManager::saveConfig(ConfigMode mode)
{
    const char *prefix = getKeyPrefix(mode);
    EffectConfig &cfg = configs[(uint8_t)mode];

    char key[8];

    sprintf(key, "%s_mh", prefix);
    prefs.putUChar(key, cfg.mainHue);

    sprintf(key, "%s_ms", prefix);
    prefs.putUChar(key, cfg.mainSat);

    sprintf(key, "%s_sh", prefix);
    prefs.putUChar(key, cfg.secondaryHue);

    sprintf(key, "%s_ss", prefix);
    prefs.putUChar(key, cfg.secondarySat);

    sprintf(key, "%s_spd", prefix);
    prefs.putUChar(key, cfg.speed);

    sprintf(key, "%s_int", prefix);
    prefs.putUChar(key, cfg.intensity);

    sprintf(key, "%s_sec", prefix);
    prefs.putBool(key, cfg.secondaryEnabled);
}
