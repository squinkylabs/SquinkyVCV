
#include "FFTCrossFader.h"

NoiseMessage* FFTCrossFader::step(float* out)
{
    *out = 0;
    return nullptr;
}

NoiseMessage * FFTCrossFader::acceptData(NoiseMessage*)
{
    return nullptr;
}
