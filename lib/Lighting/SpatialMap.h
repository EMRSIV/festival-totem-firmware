#pragma once
#include <vector>
#include <math.h>

struct Vec3
{
    float x, y, z;
};

class SpatialMap
{
public:
    SpatialMap(uint16_t totalLEDs, uint8_t ledSegments,
               float radiusCm, float spacingCm,
               bool clockwise = true);

    void begin();
    const Vec3 &pos(uint16_t index) const { return coords[index]; }
    uint16_t count() const { return totalLEDs; }

private:
    uint16_t totalLEDs;
    uint8_t ledStringSegments;
    float radius;
    float spacing;
    bool cw;

    std::vector<Vec3> coords;
};
