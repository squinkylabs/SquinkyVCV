
/** MinBLEP VCO with subharmonics.
 * based on the Fundamental VCO 1.0
 */

#pragma once

#ifndef _MSC_VER 
#include "simd.h"
#include "SimdBlocks.h"
#include "dsp/minblep.hpp"
#include "dsp/approx.hpp"
#include "dsp/filter.hpp"

using namespace rack;		// normally I don't like "using", but this is third party code...
extern bool _logvco;


/**
 * T is the sample type (usually float or float_4)
 * I is the integer type (usually int or int32_4)
 */
template <int OVERSAMPLE, int QUALITY, typename T, typename I>
class VoltageControlledOscillator
{
public:
	int index=-1;		// just for debugging

	/**
	 * sets the waveformat for main VCO and for the two subs
	 * parameters are proper mask vectors
	 */
	void setWaveform(T mainSaw, T subSaw);

	/**
	 * Set num channels and all pitches for VCO.
	 * note that most of the parameters are vectors
	 */
	void setupSub(int channels, T pitch, I subDivisorA, I subDivisorB);

	void process(float deltaTime, T syncValue);
	T main() const {
		return mainValue;
	}
    T sub(int side) const {
		return subValue[side];
	}

	int _channels = 0;
private:
	using MinBlep = dsp::MinBlepGenerator<QUALITY, OVERSAMPLE, T>;
	MinBlep mainMinBlep;
	MinBlep subMinBlep[2];
	T mainValue = 0;

	// subs are all two element arrays A and B
	I subCounter[2] = {I(1), I(1)};
	I subDivisionAmount[2] = {I(100), I(100)};
	T subValue[2] = { T(0), T(0)};
	T subFreq[2] = {T(0), T(0)};			// freq /subdivamount
	T subPhase[2] = {0, 0};			// like phase, but for the subharmonic

	T mainIsSaw = float_4::mask();
	T mainIsNotSaw = 0;
	T subIsSaw = float_4::mask();
	T subIsNotSaw = 0;

	bool syncEnabled = false;


	T lastSyncValue = 0.f;
	T mainPhase = 0.f;
	
	T freq;
	T pulseWidth = 0.5f;
	const T syncDirection = 1.f;

	static T saw(T phase, T minBlepValue);
	static T sqr(T phase, T minBlepValue);
	void doSquareLowToHighMinblep(T deltaPhase, T phase, T notSaw, MinBlep& minBlep, int id) const;

};

template <int OV, int Q, typename T, typename I>
inline void VoltageControlledOscillator<OV, Q, T, I>::setupSub(int channels, T pitch, I subDivisorA, I subDivisorB)
{
	//	printf("\n********* in setup sub index = %d\n", index);
	assert(index >= 0);

	freq = dsp::FREQ_C4 * dsp::approxExp2_taylor5(pitch + 30) / 1073741824;
#if 0
	printf("\nin setup sub[%d] freq = %s\nfrom pitch= %s\n",
		index,
		toStr(freq).c_str(),
		toStr(pitch).c_str());
#endif
	_channels = channels;
	assert(channels >= 0 && channels <= 4);
	simd_assertGT(subDivisorA, int32_4(0));
	simd_assertLE(subDivisorA, int32_4(16));
	simd_assertGT(subDivisorB, int32_4(0));
	simd_assertLE(subDivisorB, int32_4(16));
	subDivisionAmount[0] = subDivisorA;
	subDivisionAmount[1] = subDivisorB;

	// TODO: this reset here is what glitche, yes?
	//subCounter[0] = SimdBlocks::ifelse( subCounter[0] < 1, 1, subCounter[0]);
	//subCounter[1] = SimdBlocks::ifelse( subCounter[1] < 1, 1, subCounter[1]);

	subFreq[0] = freq / subDivisionAmount[0];
	subFreq[1] = freq / subDivisionAmount[1];
	
	// Let's keep sub from overflowing by
	// setting freq of unused ones to zero
	for (int i = 0; i < 4; ++i) {
		if (i >= channels) {
			subFreq[0][i] = 0;
			subFreq[1][i] = 0;
			subPhase[0][i] = 0;
			subPhase[1][i] = 0;
		}
	}
}

template <int OV, int Q, typename T, typename I>
inline void VoltageControlledOscillator<OV, Q, T, I>::setWaveform(T mainSaw, T subSaw)
{
	simd_assertMask(mainSaw);
	simd_assertMask(subSaw);
//	printf("mainisSw = %d for %p\n", mainSaw, this); fflush(stdout);
	mainIsSaw = mainSaw;
	subIsSaw = subSaw;

	mainIsNotSaw = mainIsSaw ^ float_4::mask();
	subIsNotSaw = subIsSaw ^ float_4::mask();
}

// FKA doSquareHighToLowMinblep 
template <int OV, int Q, typename T, typename I>
inline void VoltageControlledOscillator<OV, Q, T, I>::doSquareLowToHighMinblep(T deltaPhase, T phase, T notSaw,
	MinBlep& minBlep, int id) const
{
	T pulseCrossing = (pulseWidth + deltaPhase - phase) / deltaPhase;
	int pulseMask = simd::movemask((0 < pulseCrossing) & (pulseCrossing <= 1.f));
	if (pulseMask) {
		for (int i = 0; i < _channels; i++) {
			if (pulseMask & (1 << i)) {
				T highToLowMask = simd::movemaskInverse<T>(1 << i);
				// mask &= mainIsNotSaw;
				const T mainHighToLowMask = highToLowMask & notSaw;
				float p = pulseCrossing[i] - 1.f;
				T x = mainHighToLowMask & (2.f * syncDirection);
				minBlep.insertDiscontinuity(p, x);
			}
		}
	}	
}

template <int OV, int Q, typename T, typename I>
inline void VoltageControlledOscillator<OV, Q, T, I>::process(float deltaTime, T syncValue)
{
	assert(_channels > 0);

	// compute delta phase for this processing tick
	// TODO: do we need to do all this clamping every time?
	T deltaPhase = simd::clamp(freq * deltaTime, 1e-6f, 0.35f);
	T deltaSubPhase[2];
	deltaSubPhase[0] = simd::clamp(subFreq[0] * deltaTime, 1e-6f, 0.35f);
	deltaSubPhase[1] = simd::clamp(subFreq[1] * deltaTime, 1e-6f, 0.35f);

	// Advance the phase of everyone.
	// Don't wrap any - we will do them all at once later

	mainPhase += deltaPhase;

	simd_assertLE(mainPhase, float_4(2));	
	simd_assertGT(mainPhase, float_4(0));
	
	subPhase[0] += deltaSubPhase[0];
	subPhase[1] += deltaSubPhase[1];

	// Now might be a good time to add min-bleps for square waves

# if 1 // doing here makes the main square look right
		// it does what's the leading edge on the scope
		doSquareLowToHighMinblep(deltaPhase, mainPhase, mainIsNotSaw, mainMinBlep, 100);
		doSquareLowToHighMinblep(deltaSubPhase[0], subPhase[0], subIsNotSaw, subMinBlep[0], 101);
		doSquareLowToHighMinblep(deltaSubPhase[1], subPhase[1], subIsNotSaw, subMinBlep[1], 102);
#endif


//	printf("phase = %s\n", toStr(phase).c_str());

// orig
//		T halfCrossing = (0.5f - (phase - deltaPhase)) / deltaPhase;	
//		int halfMask = simd::movemask((0 < halfCrossing) & (halfCrossing <= 1.f));

	// When the main phase goes past one - time to re-sync eveyrone


//	T phaseOverflow = (phase > T(1));
//	simd_assertMask(phaseOverflow);
//	int phaseOverflowMask = simd::movemask(phaseOverflow);

	// If main saw crosses 1.0 in this sample, then oneCrossing will be between
	// zero and 1. It is isn't, then it doesn't cross this sample

	T oneCrossing = (1.f - (mainPhase - deltaPhase)) / deltaPhase;	


	// of 0 crossed, will have 1
	// 1 -> 2
	// 2 -> 4
	// 3 -> 8
	int oneCrossMask =  simd::movemask((0 < oneCrossing) & (oneCrossing <= 1.f));

	if (oneCrossMask) {
		for (int channelNumber = 0; channelNumber < _channels; channelNumber++) {

			if (oneCrossMask & (1 << channelNumber)) {
				T crossingMask = simd::movemaskInverse<T>(1 << channelNumber);

				// do we need saw?
				//T sawCrossingMask = crossingMask & mainIsSaw;
				float p = oneCrossing[channelNumber] - 1.f;

				// used to only do for saw, since square has own case.
				//T x = sawCrossingMask & (-2.f * syncDirection);
				// TODO: are we still generating -1..+1? why not...?
				// not that even so instead of 2 is should be 2 -phase or something
				T x =  crossingMask & (-2.f * syncDirection);
				mainMinBlep.insertDiscontinuity(p, x);

				for (int subIndex = 0; subIndex <= 1; ++subIndex) {
					assertGT(subCounter[subIndex][channelNumber], 0);
					subCounter[subIndex][channelNumber]--;
					if (subCounter[subIndex][channelNumber] == 0) {
						subCounter[subIndex][channelNumber] = subDivisionAmount[subIndex][channelNumber];

						// note: this value of "2" is a little inaccurate for subs.
						// almost the same at low-normal freq

					// this is perfect for saw
					//	T xs = crossingMask & ((-2.f ) * subPhase[subIndex]);
					 	T xs = crossingMask & T(-2.f);
						subPhase[subIndex][channelNumber] = 0;
						subMinBlep[subIndex].insertDiscontinuity(p, xs);
					}
				}
			}
		}
	}

	// We used to do this "wrap around" logic only when oneCrossMask told us to.
	// But every now and then one would slip through the cracks. So for now we
	// run it every time. We could do something smarter, if we need to.

	// old comment:  all the saws that overflow get set to zero
	// we can do scalar, above, and probably save CPU
	const T overflowMask = (mainPhase > T(1));
	mainPhase = SimdBlocks::ifelse(overflowMask, mainPhase - 1, mainPhase);
	simd_assertLE(mainPhase, float_4(1));

	const T mainBleps = mainMinBlep.process();
	const T subBleps0 = subMinBlep[0].process();
	const T subBleps1 = subMinBlep[1].process();

	mainValue = SimdBlocks::ifelse(mainIsSaw, saw(mainPhase, mainBleps), sqr(mainPhase, mainBleps));
	subValue[0] = SimdBlocks::ifelse(subIsSaw, saw(subPhase[0], subBleps0), sqr(subPhase[0], subBleps0));
	subValue[1] = SimdBlocks::ifelse(subIsSaw, saw(subPhase[1], subBleps1), sqr(subPhase[1], subBleps1));

	simd_assertLT(mainPhase, float_4(1.5));
}

template <int OV, int Q, typename T, typename I>
inline T VoltageControlledOscillator<OV, Q, T, I>::saw(T phase, T blepValue)
{
	return (phase * 2.f - 1.f) + blepValue;
}

template <int OV, int Q, typename T, typename I>
inline T VoltageControlledOscillator<OV, Q, T, I>::sqr(T phase, T blepValue)
{
	T temp = SimdBlocks::ifelse(phase > .5f, T(1.f), T(-1.f));
	return temp + blepValue;
}

#endif

