
#include "AudioMath_4.h"

#include "ObjectCache.h"
#include "SimdBlocks.h"
//#include "LookupTable.h"
#include <memory>

AudioMath_4::ScaleFun AudioMath_4::makeScalerWithBipolarAudioTrim(float x0, float x1, float y0, float y1)
{
    // Use a cached singleton for the lookup table - don't need to have unique copies
    std::shared_ptr<LookupTableParams<float>> lookup = ObjectCache<float>::getBipolarAudioTaper();
  //  const float x0 = -5;
  //  const float x1 = 5;
    const float a = (y1 - y0) / (x1 - x0);
    const float b = y0 - a * x0;

    // Notice how lambda captures the smart pointer. Now
    // lambda owns a reference to it.
    return [a, b, lookup, x0, x1](float_4 cv, float knob, float trim) {
        auto mappedTrim = LookupTable<float>::lookup(*lookup, trim);
        float_4 x = cv * mappedTrim + knob;
        // printf("in lambda, knob=%f x=%f", knob, x[0]);
        x = rack::simd::clamp(x, x0, x1);
        return a * x + b;
    };
}