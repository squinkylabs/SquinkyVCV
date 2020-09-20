#pragma once

#include "SchmidtTrigger.h"
#include <assert.h>

class ClockRecovery
{
public:
    ClockRecovery();
    /**
     * process one input sample.
     * return true if new period/
     */
    bool step(float);

    /**
     * Just for debugging
     */
    int _getResetCount() const;  
    float _getFrequency()const; 
    float _getEstimatedFrequency() const;    
private:

    SchmidtTrigger trigger;
  //  bool estimatorLocked = false;
    int estimatedPeriod = 0;
    bool lastInput = false;
    int samplesSinceLastClock = 0;
};

inline ClockRecovery::ClockRecovery() : trigger(-1, 1)
{

}

inline bool ClockRecovery::step(float finput)
{
    samplesSinceLastClock++;
    bool bInput = trigger.go(finput);
    if (bInput == lastInput) {
        // do nothing if no change
        return false;
    }
    lastInput = bInput;
    if (!bInput) {
        // ignore high to low edge
        return false;
    }

    if (samplesSinceLastClock < 3) {
        // makes unit tests work (ignores first low to high.
        // but this isn't a bad idea anyway - make it bigger, though.
        samplesSinceLastClock = 0;
        return false;
    }

    estimatedPeriod = samplesSinceLastClock;

    // later we won't always do this - depends on pll
    return true;
}

inline int ClockRecovery::_getResetCount() const 
{
    return 0;
}

inline float ClockRecovery::_getFrequency() const
{
   // return 1.f / float(estimatedPeriod);
    return 0;
}

inline float ClockRecovery::_getEstimatedFrequency() const
{
    return estimatedPeriod ? (1.f / float(estimatedPeriod)) : 0;
} 
