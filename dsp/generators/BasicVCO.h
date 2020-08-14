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
    void setPitch(float_4 f, float sampleTime);

    using  pfunc = float_4 (BasicVCO::*)(float deltaTime);
    pfunc getProcPointer();
private:
    dsp::MinBlepGenerator<16, 16, float_4> minBlep;
    float_4 phase = {};
    float_4 freq = {};
    Waveform wf = Waveform::SIN;
    float_4 sawOffsetDCComp = {};
    float_4 pulseWidth = 0.5f;

    float_4 processSaw(float deltaTime);
    float_4 processSin(float deltaTime);
    float_4 processPulse(float deltaTime);

};

inline void BasicVCO::setWaveform(Waveform w)
{
    wf = w;
}

inline void BasicVCO::setPitch(float_4 pitch, float sampleTime)
{
    // TODO: clamp / limit
  //  float highPitchLimit = sampleRate * .47f;
   
	freq = dsp::FREQ_C4 * dsp::approxExp2_taylor5(pitch + 30) / 1073741824;

    const float sawCorrect = -5.698;
    const float_4 normalizedFreq = float_4(sampleTime) * freq;
    sawOffsetDCComp = normalizedFreq * float_4(sawCorrect);
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
            break;
        case Waveform::SAW:
            ret = processSaw;
            break;
         case Waveform::SQUARE:
            ret = processPulse;
            break;
    } 
    return ret;
}

inline float_4 BasicVCO::processPulse(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;

    // TODO: we should do floor here
  //  phase = SimdBlocks::ifelse( (phase > 1), (phase - 1), phase);

    // TODO
    const int channels = 4;
    const float_4 syncDirection = 1.f;

    // from sub, doSquareLowToHighMinblep
    float_4 pulseCrossing = (pulseWidth + deltaPhase - phase) / deltaPhase;
	int pulseMask = simd::movemask((0 < pulseCrossing) & (pulseCrossing <= 1.f));
	if (pulseMask) {
		for (int i = 0; i < channels; i++) {
			if (pulseMask & (1 << i)) {
				float_4 highToLowMask = simd::movemaskInverse<float_4>(1 << i);
				// mask &= mainIsNotSaw;
				const float_4 mainHighToLowMask = highToLowMask;
				float p = pulseCrossing[i] - 1.f;
				float_4 x = mainHighToLowMask & (2.f * syncDirection);
				minBlep.insertDiscontinuity(p, x);
                // we hit this
             //   printf("low to hign\n");  fflush(stdout);
			}
		}
	}

    float_4 oneCrossing = (1.f - (phase - deltaPhase)) / deltaPhase;	
    int oneCrossMask =  simd::movemask((0 < oneCrossing) & (oneCrossing <= 1.f));

	if (oneCrossMask) {
		for (int channelNumber = 0; channelNumber < channels; channelNumber++) {
			if (oneCrossMask & (1 << channelNumber)) {
				float_4 crossingMask = simd::movemaskInverse<float_4>(1 << channelNumber);

				// do we need saw?
				//T sawCrossingMask = crossingMask & mainIsSaw;
				float p = oneCrossing[channelNumber] - 1.f;

				// used to only do for saw, since square has own case.
				//T x = sawCrossingMask & (-2.f * syncDirection);
				// TODO: are we still generating -1..+1? why not...?
				// not that even so instead of 2 is should be 2 -phase or something
				float_4 x =  crossingMask & (-2.f * syncDirection);
				minBlep.insertDiscontinuity(p, x);
            }
        }
    }

    phase -= simd::floor(phase);


    const float_4 blepValue = minBlep.process();
   	float_4 temp = SimdBlocks::ifelse(phase > pulseWidth, float_4(1.f), float_4(-1.f));
	return temp + blepValue;
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
    float_4 rawSaw = phase + float_4(.5f);
    rawSaw -= simd::trunc(rawSaw);
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