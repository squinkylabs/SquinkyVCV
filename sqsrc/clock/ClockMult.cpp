
#include "ClockMult.h"



int ClockMult::sampleClock()
{
    return 1;
}

/**
* Sends one reference tick to the multiplier
*/
void ClockMult::refClock()
{

}

/**
* When a ref count comes in early, instead of puking out a ton of
* sample clocks to keep up, we instead reset. Will immediately clear after call.
*/
bool ClockMult::getReset()
{
    return false;
}

bool ClockMult::getMultipliedClock()
{
    return false;
}

void ClockMult::setDivisor(int)
{
    
}