#include "SpatialMap.h"

SpatialMap::SpatialMap(uint16_t leds, uint8_t ledSegments,
                       float radiusCm, float spacingCm,
                       bool clockwise)
    : totalLEDs(leds),
      ledStringSegments(ledSegments),
      radius(radiusCm),
      spacing(spacingCm),
      cw(clockwise)
{
    coords.resize(totalLEDs);
}

void SpatialMap::begin()
{
    // Dynamic calculation based on total LEDs and segment count
    uint8_t segmentSize = totalLEDs / ledStringSegments;
    float angleStep = (2.0f * (float)M_PI) / ledStringSegments;
    if (!cw)
        angleStep = -angleStep;

    for (uint16_t i = 0; i < totalLEDs; ++i)
    {
        uint8_t segment = i / segmentSize; // Which U-string (0 to ledStringSegments-1)
        uint8_t offset = i % segmentSize;  // Position in string (0 to segmentSize-1)

        // XY: Position around circle
        float angle = segment * angleStep;
        float x = radius * cos(angle);
        float y = radius * sin(angle);

        // Z: U-shape vertical mapping
        // LED 0 and LED (segmentSize-1) are at hole level (z=0)
        // Middle LEDs are at the lowest point
        int halfPoint = segmentSize / 2;
        int depthLevel;

        if (offset < halfPoint)
        {
            // Going down: first half of string
            depthLevel = offset;
        }
        else
        {
            // Coming back up: second half of string
            depthLevel = (segmentSize - 1) - offset;
        }

        float z = -spacing * depthLevel;

        coords[i] = {x, y, z};
    }
}
