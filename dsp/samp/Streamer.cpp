
#pragma once

#include "Streamer.h"

float Streamer::step() 
{
    //int curIntegerSampleOffset = 0;
    float ret = 0;
    if (curIntegerSampleOffset < (frames-1)) {
        ++curIntegerSampleOffset;
    } else {
        arePlaying = false;

    }
    return 0;
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
void Streamer::setTranspose(bool doTranspoe, float amount)
{
    
}
