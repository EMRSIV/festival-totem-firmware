#pragma once
#include "Effect.h"
#include <vector>

class EffectManager
{
public:
    void add(Effect *fx, const char *name);
    bool setEffect(uint8_t id);
    bool next();
    uint8_t count() const;
    Effect *active();
    const char *activeName() const;

    void render(const LightingParams &params,
                const SpatialMap &map,
                CRGB *mainLeds,
                uint16_t mainCount,
                CRGB *detailLeds,
                uint16_t detailCount,
                uint32_t nowMs);

private:
    std::vector<Effect *> effects;
    std::vector<const char *> names;
    uint8_t current = 0;
};
