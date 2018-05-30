
#include <assert.h>
#include "ClockMult.h"
#include <stdio.h>



void ClockMult::sampleClock()
{
    printf("sampleClock: state=%d saw=%f\n", state, sawPhase);
    switch (state) {
        case State::INIT:
            break;
        case State::TRAINING:
            ++trainingCounter;
            break;
        case State::RUNNING: 
            sawPhase += learnedFrequency;
            if (clockOutTimer > 0) {
                clockOutTimer--;
            } else {
                clockOutValue = false;
                printf("clock out one-shot timed out, going low\n");
            }

            break;

        default: 
            assert(false);
    }
    printf("leave sampleClock: state=%d saw=%f\n", state, sawPhase);
}

/**
* Sends one reference tick to the multiplier
*/
void ClockMult::refClock()
{
    printf("refClock: state=%d\n", state);
    switch (state) {
        case State::INIT:
            state = State::TRAINING;
            trainingCounter = 0;
            printf("refClock moved from INIT to TRAINIG\n");
            break;
        case State::TRAINING:
            printf("got end train with ctr = %d\n", trainingCounter);
            learnedPeriod = trainingCounter;
            learnedFrequency = 1.0f / learnedPeriod;
            state = State::RUNNING;
            
            startNewClock();
            printf("refClock moved from TRAINING to RUNNING. period = %d freq=%f clockOut=%d\n",
                learnedPeriod, learnedFrequency, clockOutValue);
            break;
        default:
            assert(0);
    }
    printf("leave refClock: state=%d\n", state);
}

void ClockMult::startNewClock()
{
    clockOutValue = true;
    clockOutTimer = 10;         // TODO: constants
}


void ClockMult::setDivisor(int)
{
    
}