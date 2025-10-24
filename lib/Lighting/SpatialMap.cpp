#include "SpatialMap.h"

SpatialMap::SpatialMap(uint16_t leds, uint8_t holes,
                       float radiusCm, float spacingCm,
                       bool clockwise)
    : totalLEDs(leds),
      holeCount(holes),
      radius(radiusCm),
      spacing(spacingCm),
      cw(clockwise)
{
    coords.resize(totalLEDs);
}

void SpatialMap::begin()
{
    // 30 LEDs per segment, 10 segments → 300 LEDs
    uint8_t segmentSize = totalLEDs / holeCount;
    float angleStep = (2.0f * (float)M_PI) / holeCount;
    if (!cw)
        angleStep = -angleStep;

    for (uint16_t i = 0; i < totalLEDs; ++i)
    {
        uint8_t s = i / segmentSize;      // segment index 0..9
        uint8_t offset = i % segmentSize; // LED inside segment 0..29

        float angle = s * angleStep;

        // Position around circle in XY
        float x = radius * cos(angle);
        float y = radius * sin(angle);

        // Vertical mapping: linear down/up
        // offset 0 = hole height (z=0)
        // offset 15 = lowest (z=-45cm)
        float z = 0;
        if (offset <= 15)
        {
            z = -spacing * offset;
        }
        else
        {
            z = -spacing * (30 - offset);
        }

        coords[i] = {x, y, z};
    }
}
