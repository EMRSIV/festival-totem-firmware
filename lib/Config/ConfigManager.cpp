#include "ConfigManager.h"
#include <Arduino.h>

ConfigManager::ConfigManager()
    : activeMode(ConfigMode::Default),
      energyBurstState(EnergyBurstState::Inactive),
      bootEffectID(0)
{
    // Initialize default configs with sensible values
    // Default mode
    configs[0].mainHue = 0;
    configs[0].mainSat = 255;
    configs[0].secondaryHue = 128;
    configs[0].secondarySat = 255;
    configs[0].speed = 100;
    configs[0].intensity = 255;
    configs[0].secondaryEnabled = true;

    // Special1 - Strobe (default white)
    configs[1].mainHue = 0;
    configs[1].mainSat = 0; // White
    configs[1].speed = 128;

    // Special2 - Energy Burst
    configs[2].mainHue = 160; // Blue-ish
    configs[2].mainSat = 255;
    configs[2].speed = 150;
    configs[2].intensity = 128;

    // Special3 - Emergency
    configs[3].speed = 100;
}

void ConfigManager::begin()
{
    prefs.begin("totem", false); // Read-write mode

    // Load all configs from NVS
    for (int i = 0; i < 4; i++)
    {
        loadConfig((ConfigMode)i);
    }

    // Load boot effect ID
    bootEffectID = prefs.getUChar("boot_fx", 0);

    Serial.println("ConfigManager: Loaded from NVS");
}

void ConfigManager::save()
{
    // Save all configs
    for (int i = 0; i < 4; i++)
    {
        saveConfig((ConfigMode)i);
    }

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
