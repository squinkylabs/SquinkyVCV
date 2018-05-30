
#include <assert.h>
#include "ClockMult.h"
#include <stdio.h>



int ClockMult::sampleClock()
{
    int clocks = 0;
    switch (state) {
        case State::INIT:
            clocks = 1;
            break;
        case State::TRAINING:
            clocks = 1;
            ++trainingCounter;
            break;
        default: 
            assert(false);
    }
    return clocks;
}

/**
* Sends one reference tick to the multiplier
*/
void ClockMult::refClock()
{
    switch (state) {
        case State::INIT:
            state = State::TRAINING;
            trainingCounter = 0;
            break;
        case State::TRAINING:
            printf("got end train with ctr = %d\n", trainingCounter);
            learnedPeriod = trainingCounter;
            state = State::RUNNING;
            break;
        default:
            assert(0);
    }
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