#include "EffectManager.h"
#include <Arduino.h>

void EffectManager::add(Effect *fx, const char *name)
{
    effects.push_back(fx);
    names.push_back(name);
}

bool EffectManager::setEffect(uint8_t id)
{
    if (id < effects.size() && current != id)
    {
        current = id;
        return true; // changed
    }
    return false;
}

bool EffectManager::next()
{
    uint8_t newID = (current + 1) % effects.size();
    if (newID != current)
    {
        current = newID;
        return true;
    }
    return false;
}

uint8_t EffectManager::count() const
{
    return effects.size();
}

Effect *EffectManager::active()
{
    return effects.empty() ? nullptr : effects[current];
}

const char *EffectManager::activeName() const
{
    return effects.empty() ? "None" : names[current];
}

void EffectManager::render(const LightingParams &p,
                           const SpatialMap &s,
                           CRGB *mainLeds, uint16_t nMain,
                           CRGB *detailLeds, uint16_t nDetail,
                           uint32_t nowMs)
{
    if (!active())
        return;
    effects[current]->render(p, s, mainLeds, nMain, detailLeds, nDetail, nowMs);
}
