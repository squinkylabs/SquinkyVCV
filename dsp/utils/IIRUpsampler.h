#pragma once

#include "BiquadParams.h"
#include "BiquadState.h"
//#include "ButterworthFilterDesigner.h"
#include "BiquadFilter.h"
#include "ObjectCache.h"

template <int FACTOR>
class IIRUpsampler
{
public:
    void process(float * outputBuffer, float input)
    {
        input *= FACTOR;
        for (int i = 0; i < FACTOR; ++i) {
            outputBuffer[i] = BiquadFilter<float>::run(input, state, *params);
            input = 0;      // just filter a delta - don't average the whole signal
        }
    }
    /**
    * cutoff is normalized freq (.5 = nyquist).
    * typically cutoff will be <  (.5 / FACTOR),
    * if not, the filters wouldn't work.
    */
    void setCutoff(float cutoff)
    {
        assert(cutoff > 0 && cutoff < .5f);
       // ButterworthFilterDesigner<float>::designSixPoleLowpass(params, cutoff);
        params = ObjectCache<float>::get6PLPParams(cutoff);
    }
private:
    std::shared_ptr<BiquadParams<float, 3>> params;
    BiquadState<float, 3> state;
};