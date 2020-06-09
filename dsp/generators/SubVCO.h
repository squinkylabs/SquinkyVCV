
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

#if 1
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
private:
	dsp::MinBlepGenerator<QUALITY, OVERSAMPLE, T> mainMinBlep;
	dsp::MinBlepGenerator<QUALITY, OVERSAMPLE, T> subMinBlep[2];
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
	// For optimizing in serial code
	int _channels = 0;

	T lastSyncValue = 0.f;
	T phase = 0.f;
	
	T freq;
	T pulseWidth = 0.5f;
	const T syncDirection = 1.f;
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
	phase += deltaPhase;
	subPhase[0] += deltaSubPhase[0];
	subPhase[1] += deltaSubPhase[1];

	// Now might be a good time to add min-bleps for square waves


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

	T oneCrossing = (1.f - (phase - deltaPhase)) / deltaPhase;	


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
				T x =  oneCrossMask & (-2.f * syncDirection);
				mainMinBlep.insertDiscontinuity(p, x);

				

				
			//	printf("insert main discont\n");

			//	if (_logvco) {
			//		printf("** insert disc(%f, %f)\n", p, x[0]);
			//	}
				// Implement the new  dual subs
				for (int subIndex = 0; subIndex <= 1; ++subIndex) {
					assertGT(subCounter[subIndex][channelNumber], 0);
					subCounter[subIndex][channelNumber]--;
					if (subCounter[subIndex][channelNumber] == 0) {
						subCounter[subIndex][channelNumber] = subDivisionAmount[subIndex][channelNumber];

					//	T xs = crossingMask & ((-1.f ) * subPhase[subIndex]);
						subPhase[subIndex][channelNumber] = 0;

						// just an experiment
					//	subPhase[subIndex] += deltaSubPhase[subIndex];
				//		subPhase[0] += deltaSubPhase[0];
//	subPhase[1] += deltaSubPhase[1];

						// let's ignore min blep for now
						//subMinBlep[subIndex].insertDiscontinuity(p, xs);


				//		 printf("insert sub %d discont\n", subIndex);
					}
				}

			//	T mainSquare
			}
		}

		// all the saws that overflow get set to zero
		const T overflowMask = (phase > T(1));
		phase = SimdBlocks::ifelse(overflowMask, phase - 1, phase);
	}

	mainValue = phase * 2.f - 1.f;
	//mainValue += mainMinBlep.process();

}
#else

/*
	if (halfMask) {
			// TODO: can _channels be  >= 4? If so, I don't think this will work
			for (int channelNumber = 0; channelNumber < _channels; channelNumber++) {
				if (_logvco) {
					printf("i=%d, <<=%d and=%d\n", channelNumber, 1 << channelNumber,  (halfMask & (1 << channelNumber)));
				}
				if (halfMask & (1 << channelNumber)) {
					T crossingMask = simd::movemaskInverse<T>(1 << channelNumber);

					// do we need saw?
					T sawCrossingMask = crossingMask & mainIsSaw;
					float p = halfCrossing[channelNumber] - 1.f;

					// used to only do for saw, since square has own case.
					//T x = sawCrossingMask & (-2.f * syncDirection);
					T x =  crossingMask & (-2.f * syncDirection);
					mainMinBlep.insertDiscontinuity(p, x);
				//	printf("insert main discont\n");

					if (_logvco) {
						printf("** insert disc(%f, %f)\n", p, x[0]);
					}
					// Implement the new  dual subs
					for (int subIndex = 0; subIndex <= 1; ++subIndex) {
						assertGT(subCounter[subIndex][channelNumber], 0);
						subCounter[subIndex][channelNumber]--;
						if (subCounter[subIndex][channelNumber] == 0) {
							subCounter[subIndex][channelNumber] = subDivisionAmount[subIndex][channelNumber];

							T xs = crossingMask & ((-1.f ) * subPhase[subIndex]);
							subPhase[subIndex][channelNumber] = 0;

							// just an experiment
						//	subPhase[subIndex] += deltaSubPhase[subIndex];
					//		subPhase[0] += deltaSubPhase[0];
	//	subPhase[1] += deltaSubPhase[1];
					 		subMinBlep[subIndex].insertDiscontinuity(p, xs);
					//		 printf("insert sub %d discont\n", subIndex);
						}
					}

				//	T mainSquare
				}
			}
		}
*/



//#if 0 // old version

// Accurate only on [0, 1]
template <typename T>
T sin2pi_pade_05_7_6(T x) {
	x -= 0.5f;
	return (T(-6.28319) * x + T(35.353) * simd::pow(x, 3) - T(44.9043) * simd::pow(x, 5) + T(16.0951) * simd::pow(x, 7))
	       / (1 + T(0.953136) * simd::pow(x, 2) + T(0.430238) * simd::pow(x, 4) + T(0.0981408) * simd::pow(x, 6));
}

template <typename T>
T sin2pi_pade_05_5_4(T x) {
	x -= 0.5f;
	return (T(-6.283185307) * x + T(33.19863968) * simd::pow(x, 3) - T(32.44191367) * simd::pow(x, 5))
	       / (1 + T(1.296008659) * simd::pow(x, 2) + T(0.7028072946) * simd::pow(x, 4));
}

// TODO: move out of global scope
template <typename T>
T expCurve(T x) {
	return (3 + x * (-13 + 5 * x)) / (3 + 2 * x);
}

/**
 * T is the sample type (usually float or float_4)
 * I is the integer type (usually int or int32_4)
 */
template <int OVERSAMPLE, int QUALITY, typename T, typename I>
struct VoltageControlledOscillator
{

	bool analog = false;
	bool soft = false;
	bool syncEnabled = false;
	// For optimizing in serial code
	int _channels = 0;

	T lastSyncValue = 0.f;
	T phase = 0.f;
	T subPhase[2] = {0, 0};			// like phase, but for the subharmonic
	T freq;
	T pulseWidth = 0.5f;
	const T syncDirection = 1.f;

	rack::dsp::TRCFilter<T> sqrFilter;

	dsp::MinBlepGenerator<QUALITY, OVERSAMPLE, T> mainMinBlep;
	dsp::MinBlepGenerator<QUALITY, OVERSAMPLE, T> subMinBlep[2];

	T mainValue = 0.f;	// square or saw, depends on waveform setting

	// subs are all two element arrays A and B
	I subCounter[2] = {I(1), I(1)};
	I subDivisionAmount[2] = {I(100), I(100)};
	T subValue[2] = { T(0), T(0)};
	T subFreq[2] = {T(0), T(0)};			// freq /subdivamount

	T mainIsSaw = float_4::mask();
	T mainIsNotSaw = 0;
	T subIsSaw = float_4::mask();
	T subIsNotSaw = 0;

	int debugCtr = 0;
	int index=-1;		// just for debugging

	void setWaveform(T mainSaw, T subSaw)
	{
		simd_assertMask(mainSaw);
		simd_assertMask(subSaw);
	//	printf("mainisSw = %d for %p\n", mainSaw, this); fflush(stdout);
		mainIsSaw = mainSaw;
		subIsSaw = subSaw;

		mainIsNotSaw = mainIsSaw ^ float_4::mask();
		subIsNotSaw = subIsSaw ^ float_4::mask();
	#if 0
	printf("setWaveform mainisSw = %s\n  subSaw= %s\n !mainsaw=%s\n, !mainsub=%s\n", 
		toStr(mainIsSaw).c_str(),
		toStr(subIsSaw).c_str(),
		toStr(mainIsNotSaw).c_str(),
		toStr(subIsNotSaw).c_str());
		fflush(stdout);

		
		simd_assertMask(mainIsSaw);
		simd_assertMask(subIsSaw);
		simd_assertMask(mainIsNotSaw);
		simd_assertMask(subIsNotSaw);
#endif		
	}


	void setupSub(int channels, T pitch, I subDivisorA, I subDivisorB)
	{
	//	printf("\n********* in setup sub index = %d\n", index);
		assert(index >= 0);
		static int printCount = 20;		// turn off printing

		freq = dsp::FREQ_C4 * dsp::approxExp2_taylor5(pitch + 30) / 1073741824;
		_channels = channels;
		assert(channels >= 0 && channels <= 4);
	//	printf("channels in this sub = %d\n", channels);
		simd_assertGT(subDivisorA, int32_4(0));
		simd_assertLE(subDivisorA, int32_4(16));
		simd_assertGT(subDivisorB, int32_4(0));
		simd_assertLE(subDivisorB, int32_4(16));
		debugCtr = 0;
		subDivisionAmount[0] = subDivisorA;
		subDivisionAmount[1] = subDivisorB;

		// TODO: this reset here is what glitche, yes?
		subCounter[0] = SimdBlocks::ifelse( subCounter[0] < 1, 1, subCounter[0]);
		subCounter[1] = SimdBlocks::ifelse( subCounter[1] < 1, 1, subCounter[1]);

		subFreq[0] = freq / subDivisionAmount[0];
		subFreq[1] = freq / subDivisionAmount[1];
		
#ifndef NDEBUG
		if (printCount < 10) {
			printf(" setSub %d sub=(%s)\n", index, toStr(subDivisionAmount[0]).c_str());
			printf(" freq = %s, subFreq=%s\n", toStr(freq).c_str(), toStr(subFreq[0]).c_str());
			printf(" phase = %s, subPhase=%s\n", toStr(phase).c_str(), toStr(subPhase[0]).c_str());
			printf(" channels = %d\n", channels);
		}
#endif
		
		// Let's keep sub from overflowing by
		// setting freq of unused ones to zero
		for (int i = 0; i < 4; ++i) {
			if (i >= channels) {
				subFreq[0][i] = 0;
				subFreq[1][i] = 0;
				subPhase[0][i] = 0;
				subPhase[1][i] = 0;
				if (printCount < 10) {
					printf("set freq to zero on vco %d\n", i);
				}
			}
		}
		if (printCount < 10) {
			printf("**** leaving setupSub\n\n");
		}
		//printf("leaving setup, counter0 = %s\n", toStr(subCounter[0]).c_str());
		//printf("leaving setup, counter1 = %s\n", toStr(subCounter[1]).c_str());
		++printCount;
	}

	void setPulseWidth(T pulseWidth) {
		const float pwMin = 0.01f;
		this->pulseWidth = simd::clamp(pulseWidth, pwMin, 1.f - pwMin);
	}

	// This was first one
	void doSquareLowToHighMinblep(T deltaPhase, T phase, T notSaw,
		dsp::MinBlepGenerator<QUALITY, OVERSAMPLE, T>& minBlep, int id) const
	{
		T wrapCrossing = (deltaPhase - phase ) / deltaPhase;
		int wrapMask = simd::movemask((0 < wrapCrossing) & (wrapCrossing <= 1.f));
		if (wrapMask) {
			for (int i = 0; i < _channels; i++) {
				if (wrapMask & (1 << i)) {
					// ok, this VCO here is wrapping
					const T lowToHighMask = simd::movemaskInverse<T>(1 << i);
					simd_assertMask(lowToHighMask);

					// "new" way of preventing saw from getting into sq minblep
					// TODO: remove the word "main" -> it's not correct any more
					const T mainLowToHighMask = lowToHighMask & notSaw;
					float p = wrapCrossing[i] - 1.f;
					T x = mainLowToHighMask & (2.f * syncDirection);
					minBlep.insertDiscontinuity(p, x);
				//	printf("insert minBlep[%d] x = %s\n", id, toStr(x).c_str());
				//	printf("index = %d, phase was %s\n", i, toStr(phase).c_str());
				}
			}
		}
	}

	void doSquareHighToLowMinblep(T deltaPhase, T phase, T notSaw,
		dsp::MinBlepGenerator<QUALITY, OVERSAMPLE, T>& minBlep, int id) const
	{
		T pulseCrossing = (pulseWidth - (phase - deltaPhase)) / deltaPhase;
		int pulseMask = simd::movemask((0 < pulseCrossing) & (pulseCrossing <= 1.f));
		if (pulseMask) {
			for (int i = 0; i < _channels; i++) {
				if (pulseMask & (1 << i)) {
					T highToLowMask = simd::movemaskInverse<T>(1 << i);
					// mask &= mainIsNotSaw;
					const T mainHighToLowMask = highToLowMask & notSaw;
					float p = pulseCrossing[i] - 1.f;
					T x = mainHighToLowMask & (-2.f * syncDirection);
				//	if (!mainIsSaw) {
						minBlep.insertDiscontinuity(p, x);
				//	}
				}
			}
		}	
	}

	void process(float deltaTime, T syncValue) {

	//	printf("process, counter0 = %s\n", toStr(subCounter[0]).c_str());
	//	printf("proces, counter1 = %s\n", toStr(subCounter[1]).c_str());
		// Advance phase
		T deltaPhase = simd::clamp(freq * deltaTime, 1e-6f, 0.35f);

		T deltaSubPhase[2];
		deltaSubPhase[0] = simd::clamp(subFreq[0] * deltaTime, 1e-6f, 0.35f);
		deltaSubPhase[1] = simd::clamp(subFreq[1] * deltaTime, 1e-6f, 0.35f);

#if 0	// doing up here make it unaligned by a sampe = very bad
		doSquareLowToHighMinblep(deltaPhase, phase, mainIsNotSaw, mainMinBlep, 100);
		doSquareLowToHighMinblep(deltaSubPhase[0], subPhase[0], subIsNotSaw, subMinBlep[0], 101);
		doSquareLowToHighMinblep(deltaSubPhase[1], subPhase[1], subIsNotSaw, subMinBlep[1], 102);

	//if (phase[0] < deltaPhase[0]) printf("phase < delta\n");
	//	if (subPhase[0][0] < deltaSubPhase[0][0]) printf("sub 0 phase < delta\n");
	//	if (subPhase[1][0] < deltaSubPhase[1][0]) printf("sub 1 phase < delta\n");
#endif

#if 0
		if (soft) {
			// Reverse direction
			deltaPhase *= syncDirection;
		}
		else {
			// Reset back to forward
			syncDirection = 1.f;
		}
	#endif

		phase += deltaPhase;
		// Wrap phase
		phase -= simd::floor(phase);

		// we don't wrap this phase - the phase sync to master osc does it for us
		subPhase[0] += deltaSubPhase[0];
		subPhase[1] += deltaSubPhase[1];

	
		// Jump sqr when crossing 0, or 1 if backwards
		// min blop for rising edge of sq
		
# if 1 // doing here makes the main square look right
		// it does what's the leading edge on the scope
		doSquareLowToHighMinblep(deltaPhase, phase, mainIsNotSaw, mainMinBlep, 100);
	//	doSquareLowToHighMinblep(deltaSubPhase[0], subPhase[0], subIsNotSaw, subMinBlep[0], 101);
	//	doSquareLowToHighMinblep(deltaSubPhase[1], subPhase[1], subIsNotSaw, subMinBlep[1], 102);
#endif

		// doing here makes the falling edge of the subs look right
#if 0
		doSquareHighToLowMinblep(deltaPhase, phase, mainIsNotSaw, mainMinBlep, 100);
		doSquareHighToLowMinblep(deltaSubPhase[0], subPhase[0], subIsNotSaw, subMinBlep[0], 101);
		doSquareHighToLowMinblep(deltaSubPhase[1], subPhase[1], subIsNotSaw, subMinBlep[1], 102);
#endif


	
#if 0 // just debuggin
		static float maxPhase = -1;
		static float minPhase = 2;
		static float maxSubPhase = -1;
		static float minSubPhase = 2;
		if (phase[0] > maxPhase) {
			maxPhase = phase[0];
		//	printf("max phase = %f\n", maxPhase);
		}
		if (phase[0] < minPhase) {
			minPhase = phase[0];
		//	printf("min phase = %f\n", minPhase);
		}
		if (subPhase[0][0] > maxSubPhase) {
			maxSubPhase = subPhase[0][0];
			// get up to almost 1
		//	printf("max subphase = %f\n", maxPhase);
		}
		if (subPhase[0][0] < minSubPhase) {
			minSubPhase =subPhase[0][0];
			// gets down to .005
			printf("min subphase = %f\n", minPhase);
		}
#endif


#if 0
		T wrapCrossing = (deltaPhase - phase ) / deltaPhase;
		int wrapMask = simd::movemask((0 < wrapCrossing) & (wrapCrossing <= 1.f));
		if (wrapMask) {
			for (int i = 0; i < _channels; i++) {
				if (wrapMask & (1 << i)) {
					// ok, this VCO here is wrapping
					const T lowToHighMask = simd::movemaskInverse<T>(1 << i);

					// "new" way of preventing saw from getting into sq minblep
					const T mainLowToHighMask = lowToHighMask & mainIsNotSaw;
					float p = wrapCrossing[i] - 1.f;
					T x = mainLowToHighMask & (2.f * syncDirection);
					mainMinBlep.insertDiscontinuity(p, x);

					//const T subLowToHighMask= lowToHighMask & subIsNotSaw;

				}
			}
		}
	#endif

		++debugCtr;

		// Jump sqr when crossing `pulseWidth`
		// falling edge (don't need this, we can do in saw corrins routine where we do others)
	#if 0
		T pulseCrossing = (pulseWidth - (phase - deltaPhase)) / deltaPhase;
		int pulseMask = simd::movemask((0 < pulseCrossing) & (pulseCrossing <= 1.f));
		if (pulseMask) {
			for (int i = 0; i < _channels; i++) {
				if (pulseMask & (1 << i)) {
					T mask = simd::movemaskInverse<T>(1 << i);
					mask &= mainIsNotSaw;
					float p = pulseCrossing[i] - 1.f;
					T x = mask & (-2.f * syncDirection);
				//	if (!mainIsSaw) {
						mainMinBlep.insertDiscontinuity(p, x);
				//	}
				}
			}
		}
	#endif

		// Jump saw when crossing 0.5	
		// Since we are basing the subs off the saw, a lot of stuff goes on here

		T halfCrossing = (0.5f - (phase - deltaPhase)) / deltaPhase;	
		int halfMask = simd::movemask((0 < halfCrossing) & (halfCrossing <= 1.f));
		if (_logvco && 0) {
			printf("phase=%f, dp=%f hCross = %f hm=%d\n", phase[0], deltaPhase[0], halfCrossing[0], halfMask);
		}
		if (_channels <= 0) {
			printf("channels %d in vco %d\n",_channels, index); fflush(stdout);
			assert(_channels > 0);
		}
		if (halfMask) {
			// TODO: can _channels be  >= 4? If so, I don't think this will work
			for (int channelNumber = 0; channelNumber < _channels; channelNumber++) {
				if (_logvco) {
					printf("i=%d, <<=%d and=%d\n", channelNumber, 1 << channelNumber,  (halfMask & (1 << channelNumber)));
				}
				if (halfMask & (1 << channelNumber)) {
					T crossingMask = simd::movemaskInverse<T>(1 << channelNumber);

					// do we need saw?
					T sawCrossingMask = crossingMask & mainIsSaw;
					float p = halfCrossing[channelNumber] - 1.f;

					// used to only do for saw, since square has own case.
					//T x = sawCrossingMask & (-2.f * syncDirection);
					T x =  crossingMask & (-2.f * syncDirection);
				//	mainMinBlep.insertDiscontinuity(p, x);
				//	printf("insert main discont\n");

					if (_logvco) {
						printf("** insert disc(%f, %f)\n", p, x[0]);
					}
					// Implement the new  dual subs
					for (int subIndex = 0; subIndex <= 1; ++subIndex) {
						assertGT(subCounter[subIndex][channelNumber], 0);
						subCounter[subIndex][channelNumber]--;
						if (subCounter[subIndex][channelNumber] == 0) {
							subCounter[subIndex][channelNumber] = subDivisionAmount[subIndex][channelNumber];

							T xs = crossingMask & ((-1.f ) * subPhase[subIndex]);
							subPhase[subIndex][channelNumber] = 0;

							// just an experiment
						//	subPhase[subIndex] += deltaSubPhase[subIndex];
					//		subPhase[0] += deltaSubPhase[0];
	//	subPhase[1] += deltaSubPhase[1];
					 		subMinBlep[subIndex].insertDiscontinuity(p, xs);
					//		 printf("insert sub %d discont\n", subIndex);
						}
					}

				//	T mainSquare
				}
			}
		}

		//printf("compute main value, saw = %s osc=%p\n", toStr(mainIsSaw).c_str(), this); fflush(stdout);

		// hard code square make eveyrone square
		// hard code saw makes both saw, but you can see that VCO A (zero) responds
		// to UI by putting the min blep in the right place
		
	//	mainValue = (mainIsSaw) ? saw(phase) : sqr(phase);
		mainValue = SimdBlocks::ifelse(mainIsSaw, saw(phase), sqr(phase));
		mainValue += mainMinBlep.process();

		//subValue[0] = (subPhase[0] * float_4(2.f)) + float_4(-1.f);
		subValue[0] = SimdBlocks::ifelse(
			subIsSaw, 
			(subPhase[0] * float_4(2.f)) + float_4(-1.f),
			sqr(subPhase[0]));
		subValue[0] += subMinBlep[0].process();

		subValue[1] =  SimdBlocks::ifelse(
			subIsSaw, 
			(subPhase[1] * float_4(2.f)) + float_4(-1.f),
			sqr(subPhase[1]));
		subValue[1] += subMinBlep[1].process();
	}

	T saw(T phase) {
		T v;
		T x = phase + 0.5f;
		x -= simd::trunc(x);
		if (analog) {
			v = -expCurve(x);
		}
		else {
			v = 2 * x - 1;
		}
		return v;
	}

	T main() {
		return mainValue;
	}
    T sub(int side) {
		assert(side >= 0 && side <= 1);
		//printf("sub() returning %s\n", toStr(subValue).c_str());
		return subValue[side];
	}

	T sqr(T phase) {
		T v = SimdBlocks::ifelse(phase < pulseWidth, T(1.f), T(-1.f));
		return v;
	}

	T light() {
		return simd::sin(2 * T(M_PI) * phase);
	}
};
#endif

#endif


