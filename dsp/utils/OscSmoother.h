#pragma once

#include "AudioMath.h"
#include "SchmidtTrigger.h"

#include <algorithm>
#include <cmath>
#include <assert.h>

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
        return (float) std::sin( acc * AudioMath::Pi * 2);
    }
    float getFrequency() const { return delta;  }
private:
    float acc = 0;
    float delta = 0;
};

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

inline float OscSmoother::step(float input) {
    // run the edge detector, look for low to high edge
    bool edge = edgeDetector.step(input);

    if (edge) {
        periodsSinceReset++;
    }

    ++samplesSinceReset; 
  //    printf("after: edge = %d, samples=%d per=%d\n", edge, samplesSinceReset, periodsSinceReset); fflush(stdout); 
    if (periodsSinceReset > 16) {
        locked = true;
        const float samplesPerCycle = float(samplesSinceReset -1) / 16.f;
        printf("captured %f samples per cycle\n", samplesPerCycle); fflush(stdout);
    //    printf("or, using minus one %f\n", float(samplesSinceReset-1) / 16.f);

        const float newPhaseInc = 1.0f / samplesPerCycle;
        vco.setPitch(newPhaseInc);

        periodsSinceReset = 0;
        samplesSinceReset = 0;
    } 
  
    return 0;
}