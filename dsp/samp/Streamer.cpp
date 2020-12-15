
#include "Streamer.h"
#include <assert.h>

float Streamer::step() 
{
    //int curIntegerSampleOffset = 0;
    float ret = 0;

    // we don't need this compare, could be arePlaying
    if (curIntegerSampleOffset < (frames)) {
        assert(arePlaying);
        ret = data[curIntegerSampleOffset];
        ++curIntegerSampleOffset;
    }
    if (curIntegerSampleOffset >= frames) {
        arePlaying = false;;
    }
    return ret;
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
}
void Streamer::setTranspose(bool doTranspose, float amount)
{
    assert(!doTranspose);
    assert(amount == 0);  
}
