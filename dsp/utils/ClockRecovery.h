#pragma once

//#include "LowpassFilter.h"
#include "AudioMath.h"
#include "SchmidtTrigger.h"

#include <algorithm>
#include <cmath>
#include <assert.h>
#include <stdio.h>


// can this move to SqMath?
#ifndef _CLAMP
#define _CLAMP
namespace std {
    inline float clamp(float v, float lo, float hi)
    {
        assert(lo < hi);
        return std::min(hi, std::max(v, lo));
    }
}
#endif

// super simple sine vco
class Svco
{
public:
    void setPitch(float pitch) {
        delta = std::clamp(pitch, 0.f, .5f);
    }
    float process() {
        acc += delta;
        if (acc > 1) {
            acc -= 1;
        }
        return (float) std::sin( acc * AudioMath::Pi * 2);
    }
    float _getFrequency() const { return delta;  }
private:
    float acc = 0;
    float delta = 0;
};

/*****************************************************************
 */

class PLL
{
public:

    void onNewPeriodEstimate(int period);
    void onNewPeriodSameFreq();
    void step(float input);

    float _getFrequency() const { return vco._getFrequency();  }
private:
   // LowpassFilterState<float> stateLPF;
   // LowpassFilterParams<float> paramsLPF;

    Svco vco;
    float baseFreq = 0;
    float errorAcc = 0;
};

inline void PLL::onNewPeriodEstimate(int period)
{
    // on new freq we just accept it and start up loop.
    baseFreq = 1.f / float(period);
    vco.setPitch(baseFreq);
}

inline void PLL::onNewPeriodSameFreq()
{
  //  float error = stateLPF.z;
  //  printf("error = %f\n", error); fflush(stdout);

    float curFreq = vco._getFrequency();
    const float loopGain = .000001f;

    float newFreq = curFreq - loopGain * errorAcc;
    printf("accumulated error = %f new freq = %f old freq = %f\n", errorAcc, newFreq, curFreq);
    errorAcc = 0;
    vco.setPitch(newFreq);

    // TODO: close the loop
   // assert(false);
}

inline void PLL::step(float input)
{
    // run vco
    const float vcoOutput = vco.process();
    // update error
   // const float error = input * vcoOutput;
    errorAcc += input * vcoOutput;

    // update error filter
   // LowpassFilter<float>::run(error, stateLPF, paramsLPF);
}


/******************************************************************
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
#if 0
class ClockRecoveryOrig
{
public:
    ClockRecoveryOrig();
    ClockRecoveryOrig(const ClockRecoveryOrig&) = delete;
    ClockRecoveryOrig& operator= (const ClockRecoveryOrig&) = delete;

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
            // We just got a period. send to pll
            pll.onNewPeriodEstimate(estimatedPeriod);
            break;
        case States::LOCKING:
         //   pllEndOfPeriod();
            pll.onNewPeriodSameFreq();
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
    return pll._getFrequency();
}
#endif


/******************************************************************
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
    Svco vco;

    void onNewPeriodEstimate(int period);
    void onNewPeriodSameFreq();
    void stepVCO();
};

inline ClockRecovery::ClockRecovery() : trigger(-1, 1)
{
}

inline ClockRecovery::States ClockRecovery::getState() const
{
    return state;
}

inline void ClockRecovery::onNewPeriodEstimate(int period)
{
    // TODO: make this work long term
    if (vco._getFrequency() == 0) {

        // totally new one
        vco.setPitch(1.f / float(estimatedPeriod));
    }
    else {
        float currentFrequency = vco._getFrequency();
        float newFrequency = 1.f / float(estimatedPeriod);
        float error = newFrequency - currentFrequency;
        
        float loopGain = .1f;
        float newVCOFrequency = currentFrequency + error * loopGain;
        printf("cur = %f new = %f, error = %f\n", currentFrequency, newFrequency, error);
        printf("new VCO = %f\n", newVCOFrequency);
        vco.setPitch(newVCOFrequency);
    }

}
inline void ClockRecovery::onNewPeriodSameFreq()
{
    assert(false);
}

inline void ClockRecovery::stepVCO()
{
    //assert(false);
    vco.process();
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
    samplesSinceLastClock = 0;

    switch (state) {
        case States::INIT:
            state = States::LOCKING;
            // We just got a period. send to pll
            onNewPeriodEstimate(estimatedPeriod);
            break;
        case States::LOCKING:
         //   pllEndOfPeriod();
          //  onNewPeriodSameFreq();

            // TODO: rationalize this crazy state stuff
            onNewPeriodEstimate(estimatedPeriod);
            break;
        default:
            assert(false);
    }
    //pll.step(finput);
    stepVCO();

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
    return vco._getFrequency();
}

inline float ClockRecovery::_getEstimatedFrequency() const
{
    return estimatedPeriod ? (1.f / float(estimatedPeriod)) : 0;
} 
