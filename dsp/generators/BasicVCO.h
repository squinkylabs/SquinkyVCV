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
    float_4 process(float deltaTime);
    void setPitch(float_4 f, float sampleRate);
private:
    dsp::MinBlepGenerator<16, 16, float_4> minBlep;
    float_4 phaseAccumulators = {};
};

inline void BasicVCO::setWaveform(Waveform)
{
}

inline void BasicVCO::setPitch(float_4 f, float sampleRate)
{

}

inline float_4 BasicVCO::process(float deltaTime)
{
    return 0;
}