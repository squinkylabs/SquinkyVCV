#pragma once

#include "BiquadParams.h"
#include "BiquadState.h"
#include "BiquadFilter.h"

/**
 * A traditional decimator, using IIR filters for interpolation
 *
 * template parameter OVERSAMPLE is the 
 * decimation rate.
 */
template <int OVERSAMPLE>
class IIRDecimator
{
public:
    float process(const float * input)
    {
        float x = 0;
        for (int i = 0; i < OVERSAMPLE; ++i) {
            x = BiquadFilter<float>::run(input[i], state, *params);
        }
        return x;
    }

    /**
     * cutoff is normalized freq (.5 = nyquist).
     * typically cutoff will be <  (.5 / OVERSAMPLE), 
     * if not, the filters wouldn't work.
     */
    void setCutoff(float cutoff)
    {
        assert(cutoff > 0 && cutoff < .5f);
        params = ObjectCache<float>::get6PLPParams(cutoff);
    }
private:
    std::shared_ptr<BiquadParams<float, 3>> params;
    BiquadState<float, 3> state;
};