
#include "ColoredNoise.h"
#include "FFTCrossFader.h"
#include <assert.h>

NoiseMessage* FFTCrossFader::step(float* out)
{
    NoiseMessage* usedMessage = nullptr;
    if (dataFrames[0] && !dataFrames[1]) {
        // just one frame - play it;
        *out = dataFrames[0]->dataBuffer->get(curPlayOffset[0]);
        advance(0);

    } else if (dataFrames[0] && dataFrames[1]) {
        // curPlayOffset1 is the index into buffer 1, but also the crossfade index
        assert(curPlayOffset[1] < crossfadeSamples);

        printf("in step, cpo=%d,%d k0=%d k1= %d\n",
            curPlayOffset[0], curPlayOffset[1],
            (crossfadeSamples - (curPlayOffset[1] + 1)),
            curPlayOffset[1]

        );

        float buffer0Value = dataFrames[0]->dataBuffer->get(curPlayOffset[0]) * 
            (crossfadeSamples - (curPlayOffset[1]+1));
        float buffer1Value = dataFrames[1]->dataBuffer->get(curPlayOffset[1]) * curPlayOffset[1];
        *out = (buffer1Value + buffer0Value) / (crossfadeSamples-1);
        advance(0);
        advance(1);
        if (curPlayOffset[1] == crossfadeSamples) {
            // finished fade, can get rid of 0
            usedMessage = dataFrames[0];
            dataFrames[0] = dataFrames[1];
            curPlayOffset[0] = curPlayOffset[1];
            dataFrames[1] = nullptr;
            curPlayOffset[1] = 0;
        }

    } else {
        *out = 0;
    }
    return usedMessage;;
}

void FFTCrossFader::advance(int index)
{
    ++curPlayOffset[index];
    if (curPlayOffset[index] >= dataFrames[index]->dataBuffer->size()) {
        curPlayOffset[index] = 0;
    }
}

NoiseMessage * FFTCrossFader::acceptData(NoiseMessage* msg)
{
    NoiseMessage* returnedBuffer = nullptr;
    if (dataFrames[0] == nullptr) {
        dataFrames[0] = msg;
        curPlayOffset[0] = 0;
    }
    else if (dataFrames[1] == nullptr) {
        dataFrames[1] = msg;
        curPlayOffset[1] = 0;
    } else {
        // we are full, just ignroe this one.
        returnedBuffer = msg;
    }
    return returnedBuffer;
}
