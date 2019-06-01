#pragma once 

/**
 * Thin wrapper around rack MinBLEP
 * to make porting easier
 */
#ifdef __V1x
class SqBlep
{
public:
    void jump(float crossing, float jump)
    {

    }
    float shift()
    {
        return 0;
    }
};
#endif

#ifndef __V1x

#include "dsp/minblep.hpp"
class SqBlep
{
public:
    SqBlep()
    {
        this->minBLEP.minblep = rack::minblep_16_32;
        this->minBLEP.oversample = 32;
    }
    void jump(float crossing, float jump)
    {
        minBLEP.jump(crossing, jump);
    }
    float shift()
    {
        return minBLEP.shift();
    }
private:
    rack::MinBLEP<16> minBLEP;
};
#endif