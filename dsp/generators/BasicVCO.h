#pragma once

#include "simd.h"
#include "SimdBlocks.h"

class BasicVCO
{
public:
    enum class Waveform
    {
        SIN,
        TRI,
        SAW,
        SQUARE,
        EVEN,
        END     // just a marker
    };

    void setWaveform(Waveform);
 //   float_4 process(float deltaTime);
    void setPitch(float_4 f, float sampleRate);

    using  pfunc = float_4 (BasicVCO::*)(float deltaTime);
    pfunc getProcPointer();
private:
    dsp::MinBlepGenerator<16, 16, float_4> minBlep;
    float_4 phase = {};
    float_4 freq = {};
     Waveform wf = Waveform::SIN;

    float_4 processSaw(float deltaTime);
    float_4 processSin(float deltaTime);

};

inline void BasicVCO::setWaveform(Waveform w)
{
    wf = w;
}

inline void BasicVCO::setPitch(float_4 pitch, float sampleRate)
{
    // TODO: clamp / limit
  //  float highPitchLimit = sampleRate * .47f;
   
	freq = dsp::FREQ_C4 * dsp::approxExp2_taylor5(pitch + 30) / 1073741824;
}

#if 0
inline float_4 BasicVCO::process(float deltaTime)
{
    return processSaw(deltaTime);
}
#endif


inline  BasicVCO::pfunc BasicVCO::getProcPointer()
{
    auto ret = processSaw;
    switch(wf) {
        case Waveform::SIN:
            ret = processSin;
    } 
    return ret;
}

inline float_4 BasicVCO::processSaw(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;
    phase = SimdBlocks::ifelse( (phase > 1), (phase - 1), phase);

    // TODO: get real num active channels
    const int relativeChannel = 4;


    float_4 halfCrossing = (0.5f - (phase -  deltaPhase)) /  deltaPhase;
    int halfMask = simd::movemask((0 < halfCrossing) & (halfCrossing <= 1.f));
    if (halfMask) {
        for (int subChannel=0; subChannel < relativeChannel; ++subChannel) {
            if (halfMask & (1 << subChannel)) {
                float_4 mask = simd::movemaskInverse<float_4>(1 << subChannel);
                float jumpPhase = halfCrossing[subChannel] - 1.f;
                float_4 jumpAmount = mask & -2.f;
                minBlep.insertDiscontinuity(jumpPhase, jumpAmount);
            }
        }
    }

    auto minBlepValue = minBlep.process();
   // output = SimdBlocks::sinTwoPi(phase * twoPi);

   // TODO: scale, normalize, dc comp
   // TODO: minblep

    float_4 rawSaw = phase + float_4(.5f);
    rawSaw -= simd::trunc(rawSaw);
    rawSaw = 2 * rawSaw - 1;
    rawSaw += minBlepValue;

    return rawSaw;
}

inline float_4 BasicVCO::processSin(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;
    phase = SimdBlocks::ifelse( (phase > 1), (phase - 1), phase);
   // output = SimdBlocks::sinTwoPi(phase * twoPi);

   // TODO: scale, normalize, dc comp
   // TODO: minblep
    const static float_4 twoPi = 2 * 3.141592653589793238;
    return SimdBlocks::sinTwoPi(phase * twoPi);
}