#pragma once


#include "SimdBlocks.h"
#include "engine/Port.hpp"
#include "dsp/approx.hpp"
#include "dsp/minblep.hpp"
#include "simd/vector.hpp"
#include "simd/functions.hpp"

#include "ObjectCache.h"
#include "simd.h"

#define _VCOJUMP

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
        SIN_CLEAN,
        TRI_CLEAN,
        END     // just a marker
    };

    void setPitch(float_4 f, float sampleTime, float sampleRate);
    void setPw(float_4);

  //  using  processFunction = float_4 (BasicVCO:: *)(float deltaTime);
    using  processFunction = float_4 (BasicVCO::*)(float deltaTime);
    processFunction getProcPointer(Waveform);

private:
    using MinBlep = rack::dsp::MinBlepGenerator<16, 16, float_4>; 
    MinBlep minBlep;
    float_4 phase = {};
    float_4 freq = {};

    float_4 sawOffsetDCComp = {};
    float_4 pulseOffsetDCComp = {};
    float_4 currentPulseWidth = 0.5f;
    float_4 nextPulseWidth = .5f;
    float_4 triIntegrator = {};
  //  float_4 lastPitch = {-100};

    /**
    * Reference to shared lookup tables.
    * Destructor will free them automatically.
    */
    std::shared_ptr<LookupTableParams<float>> sinLookup = {ObjectCache<float>::getSinLookup()};

    rack::simd::Vector<float, 4> processSaw(float deltaTime);
    rack::simd::Vector<float, 4> processSin(float deltaTime);
    rack::simd::Vector<float, 4> processPulse(float deltaTime);
    rack::simd::Vector<float, 4> processTri(float deltaTime);
    rack::simd::Vector<float, 4> processEven(float deltaTime);
    rack::simd::Vector<float, 4> processSinClean(float deltaTime);
    rack::simd::Vector<float, 4> processTriClean(float deltaTime);

    void doSquareLowToHighMinblep(float_4 samplePoint, float_4 crossingThreshold, float_4 deltaPhase);
    void doSquareHighToLowMinblep(float_4 samplePoint, float_4 crossingThreshold, float_4 deltaPhase);
};


inline void BasicVCO::setPw(float_4 pw) 
{
    nextPulseWidth = pw;
    pulseOffsetDCComp = ((pw * 2)- 1);
}

inline void BasicVCO::setPitch(float_4 pitch, float sampleTime, float sampleRate)
{
    float_4 fmax(sampleRate * .45);
    float_4 fmin(.1f);
    
	freq = rack::dsp::FREQ_C4 * rack::dsp::approxExp2_taylor5(pitch + 30) / 1073741824;
    freq = rack::simd::clamp(freq, fmin, fmax);

    const float sawCorrect = -5.698;
    const float_4 normalizedFreq = float_4(sampleTime) * freq;
    sawOffsetDCComp = normalizedFreq * float_4(sawCorrect);
}

inline  BasicVCO::processFunction BasicVCO::getProcPointer(Waveform wf)
{
    BasicVCO::processFunction ret = &BasicVCO::processSaw;
    ret = &BasicVCO::processSaw;
    switch(wf) {
        case Waveform::SIN:
            ret = &BasicVCO::processSin;
            break;
        case Waveform::SAW:
            ret = &BasicVCO::processSaw;
            break;
         case Waveform::SQUARE:
            ret = &BasicVCO::processPulse;
            break;
        case Waveform::TRI:
            ret = &BasicVCO::processTri;
            break;
        case Waveform::EVEN:
            ret = &BasicVCO::processEven;
            break;
        case Waveform::SIN_CLEAN:
            ret = &BasicVCO::processSinClean;
            break;
         case Waveform::TRI_CLEAN:
            ret =  &BasicVCO::processTriClean;
            break;
        case Waveform::END:
        default:
            ret = &BasicVCO::processEven;
            assert(false);
            break;  
    } 
    return ret;
}

class MinMaxTester
{
public:
    MinMaxTester(const std::string& name, float minLimit, float maxLimit) : 
        s(name),
        minlimit(minLimit),
        maxlimit(maxLimit)
    {

    }
    void go(float x) {
        if (x > maxb) {
         //   printf("%s new max = %f\n", s.c_str(), x); fflush(stdout);
            maxb = x;
        }
        if (x < minb) {
          //  printf("%s new min = %f\n", s.c_str(), x); fflush(stdout);
            minb= x;
        }
        assert(x > minlimit);
        assert(x < maxlimit);
    }
private:
    float maxb=-100;
    float minb=100;
     const std::string s;
    const float minlimit;
    const float maxlimit;
   
};


//static MinMaxTester tester1("output", -2.2, 2.2);

//static MinMaxTester minb1("blep1", -3.5, 3.5);
//static MinMaxTester minb2("blep2", -3.5, 3.5);


inline void BasicVCO::doSquareLowToHighMinblep(float_4 phase, float_4 crossingThreshold, float_4 deltaPhase)
{
    const float_4 syncDirection = 1.f;
    const int channels = 4;

    float_4 pulseCrossing = (crossingThreshold + deltaPhase - phase) / deltaPhase;
	int pulseMask = rack::simd::movemask((0 < pulseCrossing) & (pulseCrossing <= 1.f));
	if (pulseMask) {
		for (int i = 0; i < channels; i++) {
			if (pulseMask & (1 << i)) {
				float_4 highToLowMask = rack::simd::movemaskInverse<float_4>(1 << i);
				const float_4 mainHighToLowMask = highToLowMask;
				float p = pulseCrossing[i] - 1.f;
				float_4 x = mainHighToLowMask & (2.f * syncDirection);
				minBlep.insertDiscontinuity(p, x);
#if 0
                if (i == 0) {
                    printf("doSquareLowToHighMinblep th=%f pulseCross = %f\n", 
                        crossingThreshold[0], 
                        pulseCrossing[0]);
                    printf("phase = %f, deltaPhase = %f\n", phase[0], deltaPhase[0]);
                }
               // minb1.go(p);
#endif
			}
		}
	}
}

inline void BasicVCO::doSquareHighToLowMinblep(float_4 phase, float_4 crossingThreshold, float_4 deltaPhase)
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
#if 0
                if (channelNumber == 0) {
                    printf("doSquareHighToLowMinblep\n");
                }
              //  minb2.go(p);
#endif
            }
        }
    }
}

// from VCV Fundamental-1 VCO
template <typename T>
T sin2pi_pade_05_5_4(T x) {
	x -= 0.5f;
	return (T(-6.283185307) * x + T(33.19863968) * rack::simd::pow(x, 3) - T(32.44191367) * rack::simd::pow(x, 5))
	       / (1 + T(1.296008659) * rack::simd::pow(x, 2) + T(0.7028072946) * rack::simd::pow(x, 4));
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

    // shift from sin to cos to match waveform of EvenVCO
    float_4 shiftedSaw = phase + .25;
    shiftedSaw = SimdBlocks::ifelse( (shiftedSaw > 1) , shiftedSaw -1, shiftedSaw);
    float_4 fundamental = sin2pi_pade_05_5_4(shiftedSaw);

    doubleSaw += minBlep.process();
    doubleSaw += 2 * sawOffsetDCComp; 
    float_4 even = float_4(4 * 0.55f * 1.4f) * (doubleSaw + 1.27 * fundamental);
    return even;
}

inline float_4 BasicVCO::processTriClean(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;

    doSquareLowToHighMinblep(phase, .5f, deltaPhase);
    doSquareHighToLowMinblep(phase, 1.f, deltaPhase);
 
    phase -= rack::simd::floor(phase);
    const float_4 blepValue = minBlep.process();

   	float_4 triSquare = SimdBlocks::ifelse(phase > .5, float_4(1.f), float_4(-1.f));
    triSquare += blepValue;

    // Integrate square for triangle
    triIntegrator += 4.0 * triSquare * deltaPhase;
    triIntegrator *= (1.0 - 40.0 * deltaTime); 

    return 5.0f * 1.25f * triIntegrator;
}

inline float_4 BasicVCO::processPulse(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;

    doSquareLowToHighMinblep(phase, currentPulseWidth, deltaPhase);
    doSquareHighToLowMinblep(phase, 1, deltaPhase);
 
    currentPulseWidth = SimdBlocks::ifelse( phase >= 1, nextPulseWidth, currentPulseWidth);
    phase -= rack::simd::floor(phase);

    const float_4 blepValue = minBlep.process();
   	float_4 temp = SimdBlocks::ifelse(phase > currentPulseWidth, float_4(1.f), float_4(-1.f));
    temp += pulseOffsetDCComp;
	return 5 * .8f * (temp + blepValue);
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

    return 5 * rawSaw;
}


inline float_4 BasicVCO::processSin(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;
    phase = SimdBlocks::ifelse( (phase > 1), (phase - 1), phase);
  //  return 5 * sin2pi_pade_05_5_4(phase);

    const static float twoPi = 2 * 3.141592653589793238;
    return 5 * SimdBlocks::sinTwoPi(phase * twoPi);
}

inline float_4 BasicVCO::processSinClean(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;
    phase = SimdBlocks::ifelse( (phase > 1), (phase - 1), phase);
    float_4 output;
  
    for (int i=0; i<4; ++i) {
        output[i] = LookupTable<float>::lookup(*sinLookup, phase[i], true);
    }
    return 5 * output;
}

inline float_4 BasicVCO::processTri(float deltaTime)
{
    const float_4 deltaPhase = freq * deltaTime;
    phase += deltaPhase;
    phase = SimdBlocks::ifelse( (phase > 1), (phase - 1), phase);
    float_4 temp = 1 - 4 * rack::simd::fmin(rack::simd::fabs(phase - 0.25f), rack::simd::fabs(phase - 1.25f));
    return 5 * temp;
}
