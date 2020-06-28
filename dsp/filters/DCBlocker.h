
#pragma once

#include "ButterworthFilterDesigner.h"
#include "BiquadFilter.h"
#include "BiquadParams.h"
#include "BiquadState.h"

class DCBlocker
{
public:
    DCBlocker(float cutoffHz);
    void setSampleTime(float);
    float step(float);
private:
    bool haveSetSampleTime = false;

    BiquadParams<double, 2> dcBlockParams;
    BiquadState<double, 2> dcBlockState;
    const float cutoffHz;
};

inline DCBlocker::DCBlocker(float _cutoffHz) : cutoffHz(_cutoffHz)
{

}

inline void DCBlocker::setSampleTime(float sampleTime)
{
    haveSetSampleTime = true;
    float fcNormalized = cutoffHz * sampleTime;
    assert((fcNormalized > 0) && (fcNormalized < .1));
  
    ButterworthFilterDesigner<double>::designFourPoleHighpass(dcBlockParams, fcNormalized);
}


inline float DCBlocker::step(float input)
{
    assert(haveSetSampleTime);
    return float(BiquadFilter<double>::run(input, dcBlockState, dcBlockParams));
}