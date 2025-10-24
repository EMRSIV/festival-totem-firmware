#include "DoubleHelixEffect.h"
#include <FastLED.h>
#include <math.h>

void DoubleHelixEffect::render(const LightingParams &P,
                               const SpatialMap &M,
                               CRGB *mainLeds, uint16_t nMain,
                               CRGB *detailLeds, uint16_t nDetail,
                               uint32_t now)
{
    float t = now * (0.0005f + (P.speed * 0.000003f));

    CRGB pri = CHSV(P.mainHue, P.mainSat, P.intensity);
    CRGB sec = CHSV(P.secondaryHue, P.secondarySat, P.intensity);

    if (!P.secondaryEnabled)
        sec = pri;

    // ---- MAIN ELEMENT (2 LEDs) ----
    for (int i = 0; i < nMain; i++)
    {
        Vec3 p = M.pos(i); // usually aligned on axis but keeps symmetry
        float ang = atan2f(p.y, p.x);
        float val = sinf(ang * 2.0f + t * 2.0f);
        mainLeds[i] = (val > 0) ? pri : sec;
    }

    // ---- DETAIL STRIP (3D double helix) ----
    for (int i = 0; i < nDetail; i++)
    {
        Vec3 p = M.pos(i);
        float ang = atan2f(p.y, p.x);
        float val = sinf(ang * 2.0f + p.z * 0.12f + t * 2.0f);
        detailLeds[i] = (val > 0) ? pri : sec;
    }
}
