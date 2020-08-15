#pragma once

#include "simd.h"
#include "SimdBlocks.h"
#include "engine/Port.hpp"
#include "dsp/approx.hpp"
#include "dsp/minblep.hpp"
#include "simd/vector.hpp"

// #define _VCOJUMP

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

    void setPitch(float_4 f, float sampleTime);

#ifdef _VCOJUMP
    using  pfunc = float_4 (BasicVCO::*)(float deltaTime);
    pfunc getProcPointer();
#else
    float_4 process(float deltaTime);
#endif
private:
    using MinBlep = rack::dsp::MinBlepGenerator<16, 16, float_4>; 
    MinBlep minBlep;
    float_4 phase = {};
    float_4 freq = {};
    Waveform wf = Waveform::SIN;
    float_4 sawOffsetDCComp = {};
    float_4 pulseWidth = 0.5f;

    float_4 processSaw(float deltaTime);
    float_4 processSin(float deltaTime);
    float_4 processPulse(float deltaTime);
    float_4 processTri(float deltaTime);
    float_4 processEven(float deltaTime);

    void doSquareLowToHighMinblep(float_4 samplePoint, float_4 deltaPhase);
    void doSquareHighToLowMinblep(float_4 samplePoint, float crossingThreshold, float_4 deltaPhase);
    

};

inline void BasicVCO::setWaveform(Waveform w)
{
    wf = w;
}

inline void BasicVCO::setPitch(float_4 pitch, float sampleTime)
{
    // TODO: clamp / limit
    //  float highPitchLimit = sampleRate * .47f;
   
	freq = rack::dsp::FREQ_C4 * rack::dsp::approxExp2_taylor5(pitch + 30) / 1073741824;

    const float sawCorrect = -5.698;
    const float_4 normalizedFreq = float_4(sampleTime) * freq;
    sawOffsetDCComp = normalizedFreq * float_4(sawCorrect);
}


#ifdef _VCOJUMP
inline  BasicVCO::pfunc BasicVCO::getProcPointer()
{
    auto ret = processSaw;
    switch(wf) {
        case Waveform::SIN:
            ret = processSin;
            break;
        case Waveform::SAW:
            ret = processSaw;
            break;
         case Waveform::SQUARE:
            ret = processPulse;
            break;
        case Waveform::TRI:
            ret = processTri;
            break;
        case Waveform::EVEN:
            ret = processEven;
            break;
        case Waveform::END:
        default:
            ret = processEven;
            assert(false);
            break;  
    } 
    return ret;
}
#else
inline float_4 BasicVCO::process(float deltaTime)
{
    switch(wf) {
        case Waveform::SIN:
            return processSin(deltaTime);
            break;
        case Waveform::SAW:
            return processSaw(deltaTime);
            break;
         case Waveform::SQUARE:
            return processPulse(deltaTime);
            break;
        case Waveform::TRI:
            return processTri(deltaTime);
            break;
        case Waveform::EVEN:
            return processEven(deltaTime);
            break;
        case Waveform::END:
        default:
            return processEven(deltaTime);
            assert(false);
            break;  
    } 
    return processSaw(deltaTime);
}
#endif


inline void BasicVCO::doSquareLowToHighMinblep(float_4 phase, float_4 deltaPhase)
{
    const float_4 syncDirection = 1.f;
    const int channels = 4;

    float_4 pulseCrossing = (pulseWidth + deltaPhase - phase) / deltaPhase;
	int pulseMask = rack::simd::movemask((0 < pulseCrossing) & (pulseCrossing <= 1.f));
	if (pulseMask) {
		for (int i = 0; i < channels; i++) {
			if (pulseMask & (1 << i)) {
				float_4 highToLowMask = rack::simd::movemaskInverse<float_4>(1 << i);
				const float_4 mainHighToLowMask = highToLowMask;
				float p = pulseCrossing[i] - 1.f;
				float_4 x = mainHighToLowMask & (2.f * syncDirection);
				minBlep.insertDiscontinuity(p, x);
			}
		}
	}
}

inline void BasicVCO::doSquareHighToLowMinblep(float_4 phase, float crossingThreshold, float_4 deltaPhase)
{
    const int channels = 4;
    const float_4 syncDirection = 1;
    float_4 oneCrossing = (crossingThreshold - (phase - deltaPhase)) / deltaPhase;	
    int oneCrossMask =  rack::simd::movemask((0 < oneCrossing) & (oneCrossing <= 1.f));

	if (oneCrossMask) {
		for (int channelNumber = 0; channelNumber < channels; channelNumber++) {
			if (oneCrossMask & (1 << channelNumber)) {
				float_4 crossingMask = rack::simd::movemaskInverse<float_4>(1 << channelNumber);
				float p = oneCrossing[channelNumber] - 1.f;
				float_4 x =  crossingMask & (-2.f * syncDirection);
				minBlep.insertDiscontinuity(p, x);
            }
        }
    }
}

inline float_4 BasicVCO::processEven(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;

    // do the min blep detect before wrap
    doSquareHighToLowMinblep(phase, 1, deltaPhase);
    doSquareHighToLowMinblep(phase, .5, deltaPhase);
    phase = SimdBlocks::ifelse( (phase > 1), (phase - 1), phase);

    // double saw will fall from +1 to -1 when the saw is at .5, and at 1
    float_4 doubleSaw = SimdBlocks::ifelse((phase < 0.5) , (-1.0 + 4.0*phase) , (-1.0 + 4.0*(phase - 0.5)));

    const static float_4 twoPi = 2 * 3.141592653589793238;
    float_4 shiftedSaw = phase + .25;
    shiftedSaw = SimdBlocks::ifelse( (shiftedSaw > 1) , shiftedSaw -1, shiftedSaw);
    float_4 fundamental = SimdBlocks::sinTwoPi(shiftedSaw * twoPi);

    doubleSaw += minBlep.process();
    float_4 even = float_4(0.55f) * (doubleSaw + 1.27 * fundamental);
    return even;
}

inline float_4 BasicVCO::processPulse(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;

    doSquareLowToHighMinblep(phase, deltaPhase);
    doSquareHighToLowMinblep(phase, 1, deltaPhase);
 
    phase -= rack::simd::floor(phase);

    const float_4 blepValue = minBlep.process();
   	float_4 temp = SimdBlocks::ifelse(phase > pulseWidth, float_4(1.f), float_4(-1.f));
	return temp + blepValue;
}

inline float_4 BasicVCO::processSaw(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;

     doSquareHighToLowMinblep(phase, .5, deltaPhase);
     phase = SimdBlocks::ifelse( (phase > 1), (phase - 1), phase);


    auto minBlepValue = minBlep.process();
    float_4 rawSaw = phase + float_4(.5f);
    rawSaw -= rack::simd::trunc(rawSaw);
    rawSaw = 2 * rawSaw - 1;
    rawSaw += minBlepValue;
    rawSaw += sawOffsetDCComp;

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

inline float_4 BasicVCO::processTri(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;
    phase = SimdBlocks::ifelse( (phase > 1), (phase - 1), phase);
    return 1 - 4 * rack::simd::fmin(rack::simd::fabs(phase - 0.25f), rack::simd::fabs(phase - 1.25f));
}
