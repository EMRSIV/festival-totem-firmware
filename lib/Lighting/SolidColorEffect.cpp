#include "SolidColorEffect.h"

void SolidColorEffect::render(const LightingParams &P,
                              const SpatialMap &map,
                              CRGB *mainLeds, uint16_t nMain,
                              CRGB *detailLeds, uint16_t nDetail,
                              uint32_t)
{
    CRGB pri = CHSV(P.mainHue, P.mainSat, P.intensity);
    CRGB sec = CHSV(P.secondaryHue, P.secondarySat, P.intensity);

    if (!P.secondaryEnabled)
    {
        // Primary everywhere
        fill_solid(mainLeds, nMain, pri);
        fill_solid(detailLeds, nDetail, pri);
        return;
    }

    // ✅ Main LED Output (2px: primary, secondary)
    if (nMain > 0)
        mainLeds[0] = pri;
    if (nMain > 1)
        mainLeds[1] = sec;

    // ✅ Detail strip (10 segments x 30px)
    const uint8_t segmentSize = 30;
    const uint8_t numSegments = nDetail / segmentSize;

    for (uint8_t seg = 0; seg < numSegments; seg++)
    {
        CRGB c = (seg % 2 == 0) ? pri : sec;
        uint16_t start = seg * segmentSize;
        uint16_t end = start + segmentSize;

        for (uint16_t i = start; i < end && i < nDetail; i++)
        {
            detailLeds[i] = c;
        }
    }
}
