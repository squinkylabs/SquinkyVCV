
#include "ColoredNoise.h"
#include "FFTCrossFader.h"
#include <assert.h>

NoiseMessage* FFTCrossFader::step(float* out)
{
    if (dataFrames[0]) {
        assert(!dataFrames[1]);
        *out = dataFrames[0]->dataBuffer->get(curPlayOffset0);
        ++curPlayOffset0;
        if (dataFrames[0]->dataBuffer->size() <= curPlayOffset0) {
            curPlayOffset0 = 0;
        }
    } else {
        *out = 0;
    }
    return nullptr;
}

NoiseMessage * FFTCrossFader::acceptData(NoiseMessage* msg)
{
    if (dataFrames[0] == nullptr) {
        dataFrames[0] = msg;
        curPlayOffset0 = 0;
    } else {
        assert(false);
    }
    return nullptr;
}
