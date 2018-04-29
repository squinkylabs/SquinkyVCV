
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
        kiss_fftr_cfg newCfg=  kiss_fftr_alloc((int)in.buffer.size(),
            inverse_fft,
           nullptr, nullptr);

        assert (newCfg);
        if (!newCfg) {
            return false;
        }

        // now save off in our typeless pointer.
        assert(sizeof(newCfg) == sizeof(in.kiss_cfg));
        in.kiss_cfg = newCfg;
    }

    // step 2: do the fft
    kiss_fftr_cfg theCfg = reinterpret_cast<kiss_fftr_cfg>(in.kiss_cfg);

    // TODO: need a test that this assumption is correct (that we kiss_fft_cpx == std::complex.
    kiss_fft_cpx * outBuffer = reinterpret_cast<kiss_fft_cpx *>(out->buffer.data());
    kiss_fftr(theCfg, in.buffer.data(), outBuffer);

    // step 4: scale
    const float scale = float(1.0 / in.buffer.size());
    for (size_t i = 0; i < in.buffer.size(); ++i) {
        out->buffer[i] *= scale;
    }

    return true;
}


bool FFT::inverse(FFTDataReal* out, const FFTDataCpx& in)
{
    if (out->buffer.size() != in.buffer.size()) {
        return false;
    }

    // step 1: create the cfg, if needed
    if (in.kiss_cfg == 0) {
        bool inverse_fft = true;
        kiss_fftr_cfg newCfg = kiss_fftr_alloc((int) in.buffer.size(),
            inverse_fft,
            nullptr, nullptr);

        assert(newCfg);
        if (!newCfg) {
            return false;
        }

        // now save off in our typeless pointer.
        assert(sizeof(newCfg) == sizeof(in.kiss_cfg));
        in.kiss_cfg = newCfg;
    }

    // step 2: do the fft
    kiss_fftr_cfg theCfg = reinterpret_cast<kiss_fftr_cfg>(in.kiss_cfg);

    // TODO: need a test that this assumption is correct (that we kiss_fft_cpx == std::complex.
    const kiss_fft_cpx * inBuffer = reinterpret_cast<const kiss_fft_cpx *>(in.buffer.data());

    kiss_fftri(theCfg, inBuffer, out->buffer.data());
    return true;
}