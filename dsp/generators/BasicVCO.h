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

    float_4 processSaw(float deltaTime);
};

inline void BasicVCO::setWaveform(Waveform)
{
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
    return processSaw;
}

inline float_4 BasicVCO::processSaw(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;
    phase = SimdBlocks::ifelse( (phase > 1), (phase - 1), phase);
   // output = SimdBlocks::sinTwoPi(phase * twoPi);

   // TODO: scale, normalize, dc comp
   // TODO: minblep
    return phase;
}