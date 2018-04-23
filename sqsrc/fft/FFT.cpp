
#include "FFT.h"
#include "FFTData.h"

#include <assert.h>
#include "kiss_fft.h"
#include "kiss_fftr.h"


bool FFT::forward(FFTDataCpx* out, const FFTDataReal& in)
{
    if (out->buffer.size() != in.buffer.size()) {
        return false;
    }

    // step 1: create the cfg, if needed
    if (in.kiss_cfg == 0) {
        bool inverse_fft = false;
        size_t memsize = sizeof(in.kiss_cfg);
        kiss_fftr_cfg ret=  kiss_fftr_alloc((int)in.buffer.size(),
            inverse_fft,
            &in.kiss_cfg, &memsize);

        auto cfgsize = sizeof(kiss_fftr_cfg);
        assert (ret);
        assert(false);
    }

    // step 2: do the fft
    assert(false);
    return true;
}