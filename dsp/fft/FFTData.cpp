
#include "FFTData.h"
#include "kiss_fft.h"
#include "kiss_fftr.h"

#include <assert.h>

FFTDataCpx::FFTDataCpx(int numBins) :
    buffer(numBins)
{
}

FFTDataCpx::~FFTDataCpx()
{
    // We need to manually delete the cfg, since only "we" know
    // what type it is.
    if (kiss_cfg) {
        free(kiss_cfg);
    }
}

cpx FFTDataCpx::get(int index) const
{
    assert(index < buffer.size());
    return buffer[index];
}


void FFTDataCpx::set(int index, cpx value)
{
    assert(index < buffer.size());
    buffer[index] = value;
}

/******************************************************************/

FFTDataReal::FFTDataReal(int numBins) :
    buffer(numBins)
{
}

FFTDataReal::~FFTDataReal()
{
    // We need to manually delete the cfg, since only "we" know
    // what type it is.
    if (kiss_cfg) {
        free(kiss_cfg);
    }
}

float FFTDataReal::get(int index) const
{
    assert(index < buffer.size());
    return buffer[index];
}


void FFTDataReal::set(int index, float value)
{
    assert(index < buffer.size());
    buffer[index] = value;
}
#if 0
class FFTDataReal
{
public:
    FFTDataReal(int numBins);
    float get(int numBin);
    void set(int numBin, float value);
private:
    std::unique_ptr<float> buffer;
};
#endif