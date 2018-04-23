
#include "FFTData.h"

#include <assert.h>

FFTDataCpx::FFTDataCpx(int numBins) :
    buffer(numBins)
{

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

FFTDataReal::FFTDataReal(int numBins) :
    buffer(numBins)
{

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