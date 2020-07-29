#pragma once

#include "simd.h"
#include "SimdBlocks.h"
//#include "dsp/minblep.hpp"
#include "dsp/approx.hpp"
//#include "dsp/filter.hpp"

using namespace rack;		// normally I don't like "using", but this is third party code...

template <typename T>
class SinesVCO
{
public:
    /** Units are standard volts.
     * 0 = C4
     */
    void setPitch(T f);
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
inline void SinesVCO<T>::setPitch(T pitch)
{
	freq = dsp::FREQ_C4 * dsp::approxExp2_taylor5(pitch + 30) / 1073741824;
  //  printf("set pitch %s made freq %s\n", toStr(pitch).c_str(), toStr(freq).c_str());
  //  fflush(stdout);
}

 static float_4 twoPi = 2 * 3.141592653589793238;

template <typename T>
inline T SinesVCO<T>::process(T deltaT)
{
    /// printf("in process, freq=%s\n", toStr(freq).c_str());
    const T deltaPhase = freq * deltaT;
    phase += deltaPhase;
    // printf("in process, phase=%s\n", toStr(phase).c_str());
    phase = SimdBlocks::ifelse( (phase > 1), (phase - 1), phase);
    output = SimdBlocks::sinTwoPi(phase * twoPi);
    return output;
}