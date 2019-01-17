#pragma once 

/**
 * Thin wrapper around rack MinBLEP
 * to make porting easier
 */
#ifdef __V1
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

#ifndef __V1

class SqBlep
{
    this->minblep = minblep_16_32;
    this->oversample = 32;
};
#endif