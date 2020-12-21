
#include "Streamer.h"
#include <assert.h>

float Streamer::step() 
{
    return transposeEnabled ? stepTranspose() :  stepNoTranspose();
}

float Streamer::stepTranspose() 
{
    float ret = 0;
     assert(transposeEnabled);

    // we don't need this compare, could be arePlaying
    if (curFloatSampleOffset < (frames)) {
        assert(arePlaying);

        // TODO: interp
        const int index = int(curFloatSampleOffset);
        ret = data[index];
        curFloatSampleOffset += transposeMultiplier;
    }
    if (curFloatSampleOffset >= frames) {
        arePlaying = false;;
    }
    return ret * vol;
}

float Streamer::stepNoTranspose() 
{
    float ret = 0;
    assert(!transposeEnabled);

    // we don't need this compare, could be arePlaying
    if (curIntegerSampleOffset < (frames)) {
        assert(arePlaying);
        ret = data[curIntegerSampleOffset];
        ++curIntegerSampleOffset;
    }
    if (curIntegerSampleOffset >= frames) {
        arePlaying = false;;
    }
    return ret * vol;
}

void Streamer::mute() 
{
    vol = 0;
}
bool Streamer::canPlay() 
{
    return bool(data && arePlaying);
}

void Streamer::setSample(float* d, int f)
{
    data = d;
    frames = f;
    arePlaying = true;
    curIntegerSampleOffset = 0;
    vol = 1;
}
void Streamer::setTranspose(bool doTranspose, float amount)
{
    transposeEnabled = doTranspose;
    transposeMultiplier = amount;
}
