#pragma once

#include "simd.h"
#include "SimdBlocks.h"
//#include "dsp/minblep.hpp"
#include "dsp/approx.hpp"
//#include "dsp/filter.hpp"

using namespace rack;		// normally I don't like "using", but this is third party code...

#if 0
float_4 secondOrderApprox(float_4 x)
{
    // c=3/4=0.75
    const float c = 0.75f;

    return (2 - 4 * c) * x * x + c + x;
}
#endif

inline float_4 secondOrderApprox(float_4 phase) {
        // Quadratic approximation of sine, slightly richer harmonics
        float_4 halfPhase = (phase < 0.5f);
        float_4 x = phase - simd::ifelse(halfPhase, 0.25f, 0.75f);
        float_4 v = 1.f - 16.f * simd::pow(x, 2);
        v *= simd::ifelse(halfPhase, 1.f, -1.f);
        return v;
}

template <typename T>
class SinesVCO
{
public:
    /** Units are standard volts.
     * 0 = C4
     */
    void setPitch(T f, float sampleRate);
    T process(T deltaT);
    T get() const {
        return output;
    }
private:
    T phase = 0;
    T freq = 0;
    T output;
};

template <typename T>
inline void SinesVCO<T>::setPitch(T pitch, float sampleRate)
{
    float highPitchLimit = sampleRate * .47f;
   
	freq = dsp::FREQ_C4 * dsp::approxExp2_taylor5(pitch + 30) / 1073741824;

    for (int i=0; i<4; ++i) {
        if (freq[i] > highPitchLimit) {
            freq[i] = 0;
            phase[i] = 0;
        }
    }
}

static float_4 twoPi = 2 * 3.141592653589793238;

template <typename T>
inline T SinesVCO<T>::process(T deltaT)
{
    const T deltaPhase = freq * deltaT;
    phase += deltaPhase;
    phase = SimdBlocks::ifelse( (phase > 1), (phase - 1), phase);
    output = SimdBlocks::sinTwoPi(phase * twoPi);
    return output;
}
