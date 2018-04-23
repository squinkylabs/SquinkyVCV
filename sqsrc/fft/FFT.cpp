
#include "FFT.h"
#include "FFTData.h"

bool FFT::forward(FFTDataCpx* out, const FFTDataReal& in)
{
    if (out->buffer.size() != in.buffer.size()) {
        return false;
    }
    return true;
}