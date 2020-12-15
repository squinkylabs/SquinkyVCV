
#pragma once

#include "Streamer.h"

float Streamer::step() 
{
    return 0;
}

bool Streamer::canPlay() 
{
    return bool(data);
}

void Streamer::setSample(float* d, int f)
{
    data = d;
    frames = f;
}
void Streamer::setTranspose(bool doTranspoe, float amount)
{

}
