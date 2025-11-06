#pragma once
#include "EffectConfig.h"
#include <Preferences.h>

class ConfigManager
{
public:
    ConfigManager();

    void begin(); // Load from NVS
    void save();  // Save all configs to NVS

    // Mode switching
    void setMode(ConfigMode mode);
    ConfigMode getMode() const { return activeMode; }

    // Config access
    EffectConfig &getActiveConfig();
    EffectConfig &getConfig(ConfigMode mode);

    // Boot effect ID
    void setBootEffectID(uint8_t id);
    uint8_t getBootEffectID() const { return bootEffectID; }

    // Energy burst state management
    void setEnergyBurstState(EnergyBurstState state);
    EnergyBurstState getEnergyBurstState() const { return energyBurstState; }

private:
    EffectConfig configs[4]; // [0]=Default, [1]=Special1, [2]=Special2, [3]=Special3
    ConfigMode activeMode;
    EnergyBurstState energyBurstState;
    uint8_t bootEffectID;

    Preferences prefs;

    void loadConfig(ConfigMode mode);
    void saveConfig(ConfigMode mode);

    // NVS key helpers
    const char *getKeyPrefix(ConfigMode mode);
};
