#pragma once

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

// super simple  vco
class Svco2
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
        // we don't use sin - this is a waste.
        return (float) std::sin( acc * AudioMath::Pi * 2);
    }
    float getFrequency() const { return delta;  }
    float getTriangle() const {
        float x;
        //printf("add = %f delta=%f", acc, delta);
        if (acc < .25f) {
            x = acc * 2;
           // printf(" a: %f\n", x);
        } else if (acc < .5f) {
            x = 1 -2 * acc;
           // printf(" b: %f\n", x);
        } else if (acc < .75f) {
            x = 1 - 2 * acc;
            //printf(" c: %f\n", x);
        } else {
            x = 2 * (acc - 1);
            //printf(" d: %f\n", x);
        }
        return x;
    }

private:
    float acc = 0;
    float delta = 0;
};

/**
 * 
 * 
 * 
 */
class RisingEdgeDetector
{
public:
    RisingEdgeDetector();
    bool step(float);
private:
    SchmidtTrigger inputConditioner;
    bool lastInput = false;
};

inline RisingEdgeDetector::RisingEdgeDetector() : inputConditioner(-1, 1)
{

}

inline bool RisingEdgeDetector::step(float input)
{
    bool input2 = inputConditioner.go(input);
    if (input2 != lastInput) {
        lastInput = input2;
        if (input2) {
            return true;
        }
    }
    return false;
}

/**
 * 
 * 
 * 
 */
class RisingEdgeDetectorFractional
{
public:
    using Edge = std::pair<bool, float>;
    Edge step(float);
private:
    float lastValue = 0;
    bool wasHigh = false;
    bool wasLow = false;
};

inline RisingEdgeDetectorFractional::Edge
RisingEdgeDetectorFractional::step(float input)
{
    auto ret = std::make_pair<bool, float>(false, 0);
    if ( (input >= 0) && (lastValue < 0) && wasHigh && wasLow) {
        ret.first = true;

        float delta = input - lastValue;
	    float crossing = -lastValue / delta;
        printf("crossing, delta = %f crossing = %f\n", delta, crossing); fflush(stdout);
        ret.second = crossing;     // TODO: real fraction
        wasHigh = false;
        wasLow = false;
    } 
    
    lastValue = input;

    if (input > 1) {
        wasHigh = true;
    } else if (input < -1) {
        wasLow = true;
    }
    return ret;
}
/**
 * 
 * 
 * 
 */

class OscSmoother
{
public:
    OscSmoother();
    float step(float input);
    bool isLocked() const;
    float _getPhaseInc() const;
private:
    int cycleInCurrentGroup = 0;
    bool locked = false;
    Svco2 vco;
    RisingEdgeDetector edgeDetector;

    int periodsSinceReset = 0;
    int samplesSinceReset = 0;
};

inline OscSmoother::OscSmoother()
{
}

inline bool OscSmoother::isLocked() const {
    return locked;
}

inline float OscSmoother::_getPhaseInc() const 
{
   // return 1.f / 6.f;
   return vco.getFrequency();
}

#define PERIOD_CYCLES 16 * 4
inline float OscSmoother::step(float input) {
    // run the edge detector, look for low to high edge
    bool edge = edgeDetector.step(input);

    if (edge) {
        periodsSinceReset++;
    }

    ++samplesSinceReset; 
  //    printf("after: edge = %d, samples=%d per=%d\n", edge, samplesSinceReset, periodsSinceReset); fflush(stdout); 
    if (periodsSinceReset > PERIOD_CYCLES) {
        locked = true;
        const float samplesPerCycle = float(samplesSinceReset -1) / float(PERIOD_CYCLES);
        printf("captured %f samples per cycle %d per period\n", samplesPerCycle, samplesSinceReset); fflush(stdout);
    //    printf("or, using minus one %f\n", float(samplesSinceReset-1) / 16.f);


        // experiment - let's try constant
      //  const float newPhaseInc = 1.0f /  40.f;
        const float newPhaseInc = 1.0f / samplesPerCycle;


        vco.setPitch(newPhaseInc);

        periodsSinceReset = 0;
        samplesSinceReset = 0;
    } 
    vco.process();
    return 10 * vco.getTriangle();
}