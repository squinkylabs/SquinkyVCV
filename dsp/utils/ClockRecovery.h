#pragma once

#include "LowpassFilter.h"
#include "SchmidtTrigger.h"
#include <assert.h>


// super simple sine vco
class Svco
{
public:
    void setPitch(float pitch);
    float process();
private:
};

class PLL
{
public:
 //   PLL();

    //void inputPeriodEnded();
    void onNewPeriod(int period);
    void step(float input);
private:
    LowpassFilterState<float> stateLPF;
    LowpassFilterParams<float> paramsLPF;

    Svco vco;
};

inline void PLL::onNewPeriod(int period)
{
}

inline void PLL::step(float input)
{
}


/**
 *
 *  --- states ---
 * 
 * 0: initial - we don't know anything
 * 1: have a cycle of pitch estimation. pll is hunting
 * 2: pll locked.
 * 
 * What actions cause state to move "backwards"?
 * 
 * PLL loses lockj (2 -> 1)
 * new period estimate if far from freq
 * 
 * Freq is only valid in state2
 */

class ClockRecovery
{
public:
    ClockRecovery();
    ClockRecovery(const ClockRecovery&) = delete;
    ClockRecovery& operator= (const ClockRecovery&) = delete;

    enum class States { INIT, LOCKING, LOCKED };
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

    States getState() const;
private:

    SchmidtTrigger trigger;
    int estimatedPeriod = 0;
    bool lastInput = false;
    int samplesSinceLastClock = 0;
    States state = States::INIT;
   // Svco vco;
    PLL pll;
  //  float pllFreqOffset = 0;

  //  void pllEndOfPeriod();
};

inline ClockRecovery::ClockRecovery() : trigger(-1, 1)
{
}

inline ClockRecovery::States ClockRecovery::getState() const
{
    return state;
}

#if 0
inline void ClockRecovery::pllEndOfPeriod()
{
    if (estimatedPeriod == 0) {
        return;
    }

    float estFreqNorm = 1.f / estimatedPeriod;
    float error = 0;

}
#endif

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
    samplesSinceLastClock = 0;

    switch (state) {
        case States::INIT:
            state = States::LOCKING;
         //   pllFreqOffset = 0;
            pll.onNewPeriod(estimatedPeriod);
            break;
        case States::LOCKING:
         //   pllEndOfPeriod();
            break;
        default:
            assert(false);
    }
    pll.step(finput);

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
