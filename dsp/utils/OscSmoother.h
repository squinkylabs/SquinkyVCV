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
    float _getFrequency() const { return delta;  }
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
  //  SchmidtTrigger inputConditioner;
  // bool lastInput = false;
    RisingEdgeDetector edgeDetector;
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
    return 1.f / 6.f;
}

inline float OscSmoother::step(float input) {
    // run the edge detector, look for low to high edge
    bool input2 = edgeDetector.step(input);
#if 0
    bool input2 = inputConditioner.go(input);
    if (input2 != lastInput) {
        lastInput = input2;
        if (!input2) {
            return 0;       // todo - return real waveform
        }
    }
#endif

    ++samplesSinceReset;  
    if (samplesSinceReset > 16) {
        locked = true;
    } 
    return 0;
}