#pragma once

#include "AudioMath.h"
#include "SchmidtTrigger.h"

#include <algorithm>
#include <cmath>
#include <assert.h>
#include <stdio.h>

// #define _OSL
#define _FULL_RESET

// can this move to SqMath?
#ifndef _CLAMP
#define _CLAMP
namespace std {

    template <typename T>
    inline T clamp(T v, T lo, T hi)
    {
        assert(lo < hi);
        return std::min<T>(hi, std::max<T>(v, lo));
    }
#if 0
    inline float clamp(float v, float lo, float hi)
    {
        assert(lo < hi);
        return std::min(hi, std::max(v, lo));
    }

    inline double clamp(double v, double lo, double hi)
    {
        assert(lo < hi);
        return std::min(hi, std::max(v, lo));
    }
#endif
}
#endif

template <typename T>
inline T clamp_t(T v, T lo, T hi)
{
    assert(lo < hi);
    return std::min<T>(hi, std::max<T>(v, lo));
}

// super simple  vco
template <typename T>
class Svco2
{
public:
    void setPitch(T pitch) {
        delta = std::clamp<T>(pitch, 0.f, .5f);
    }
    void reset(T normalizedPitch, T normalizedPhase) {
        //printf("reset(%f, %f), current = %f, %f)\n", normalizedPitch, normalizedPhase, delta, acc);
       // printf("vco reset delta phase = %f delta f = %f\n", (normalizedPhase - acc), (normalizedPitch - delta)) ;
        acc = normalizedPhase;
        delta = clamp_t<T>(normalizedPitch, T(0), T(.5));
    }
    float process() {
        acc += delta;
        if (acc > 1) {
            acc -= 1;
        }
        // we don't use sin - this is a waste.
        return (float) std::sin( acc * AudioMath::Pi * 2);
    }
    T getFrequency() const { return delta;  }
    T getTriangle() const {
        T x;
        //printf("add = %f delta=%f", acc, delta);
        if (acc < .25f) {
            x = acc * 2;
        } else if (acc < .5f) {
            x = 1 -2 * acc;
        } else if (acc < .75f) {
            x = 1 - 2 * acc;
        } else {
            x = 2 * (acc - 1);
        }
        return x;
    }

private:
    T acc = 0;
    T delta = 0;
};

/**
 * 
 * 
 * 
 */
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

/**
 * 
 * 
 * 
 */
class RisingEdgeDetectorFractional
{
public:
    /**
     * x.first = bool: did cross
     * x.second = float fraction: where did it fall between prev sample and cur?
     *          0 -> it happened on this sample
     *          1 -> it happened on the last sample
     *          .5 -> it happened in between last and current
     *          .001 -> It happened right before current
     * 
     * So it is how far back in time did it cross.
     */
    using Edge = std::pair<bool, float>;
    Edge step(float);

    void _primeForTest(float last);
private:
    float lastValue = 0;
    bool wasHigh = false;
    bool wasLow = false;
};

inline  void RisingEdgeDetectorFractional::_primeForTest(float last)
{
    wasLow = true;
    wasHigh = true;
    lastValue = last;
    assert(lastValue < 0);
}

inline RisingEdgeDetectorFractional::Edge
RisingEdgeDetectorFractional::step(float input)
{
    auto ret = std::make_pair<bool, float>(false, 0);
    if ( (input >= 0) && (lastValue < 0) && wasHigh && wasLow) {
        ret.first = true;

        float delta = input - lastValue;
	    float crossing = -lastValue / delta;
       // printf("crossing, delta = %f crossing = %f\n", delta, crossing); fflush(stdout);
        assert(crossing >= 0);
        assert(crossing <= 1);

        // flip it, so small numbers are close to current sample,
        // large numerse close to prev.
        ret.second = 1 - crossing;     // TODO: real fraction
        wasHigh = false;
        wasLow = false;
    } 

    lastValue = input;

    if (input > 1) {
        wasHigh = true;
    } else if (input < -1) {
        wasLow = true;
    }
    return ret;
}
/**
 * 
 * 
 * 
 */

template <typename T>
class OscSmoother
{
public:
    OscSmoother();
    float step(float input);
    bool isLocked() const;
    T _getPhaseInc() const;
    void _primeForTest(float first) {}
private:
    int cycleInCurrentGroup = 0;
    bool locked = false;
    Svco2<T> vco;
    RisingEdgeDetector edgeDetector;

    int periodsSinceReset = 0;
    int samplesSinceReset = 0;
};

template <typename T>
inline OscSmoother<T>::OscSmoother()
{
}

template <typename T>
inline bool OscSmoother<T>::isLocked() const {
    return locked;
}

template <typename T>
inline T OscSmoother<T>::_getPhaseInc() const
{
   // return 1.f / 6.f;
   return vco.getFrequency();
}

 #define PERIOD_CYCLES_DEF 16
template <typename T>
inline float OscSmoother<T>::step(float input) {
    // run the edge detector, look for low to high edge
    bool edge = edgeDetector.step(input);

    if (edge) {
        periodsSinceReset++;
    }

    ++samplesSinceReset; 
#ifdef _OSL
    printf("after: edge = %d, samples=%d per=%d\n", edge, samplesSinceReset, periodsSinceReset); fflush(stdout); 
#endif
    if (periodsSinceReset > PERIOD_CYCLES_DEF) {
        locked = true;
        const float samplesPerCycle = float(samplesSinceReset -1) / float(PERIOD_CYCLES_DEF);
#ifdef _OSL
        printf("captured %f samples per cycle %d per period\n", samplesPerCycle, samplesSinceReset); fflush(stdout);
        printf("  actual sample count was %d, but sub 1 to %d\n", samplesSinceReset, samplesSinceReset-1);
    //    printf("or, using minus one %f\n", float(samplesSinceReset-1) / 16.f);
#endif

        // experiment - let's try constant
        //  const float newPhaseInc = 1.0f /  40.f;
        const float newPhaseInc = 1.0f / samplesPerCycle;

        vco.setPitch(newPhaseInc);
        periodsSinceReset = 0;
        samplesSinceReset = 0;
    } 
    vco.process();
    return float(10 * vco.getTriangle());
}

/**
 * 
 * 
 * 
 */
template <typename T>
class OscSmoother2
{
public:
    OscSmoother2() : smootherPeriodCycles(PERIOD_CYCLES_DEF) {}
    OscSmoother2(int periodCycles) : smootherPeriodCycles(periodCycles) {}
    float step(float input);
    bool isLocked() const;
    T _getPhaseInc() const;
    void _primeForTest(float last);
private:
    int cycleInCurrentGroup = 0;
    bool locked = false;
    Svco2<T> vco;
    RisingEdgeDetectorFractional edgeDetector;

    /**
     * just used found counting up until next to to do calculation.
     * Note required to be super accurate.
     */
    int integerPeriodsSinceReset = 0;

    /**
     */
    T fractionalSamplesSinceReset = 0;
    int integerSamplesSinceReset = 0;

    const int smootherPeriodCycles;

    // treat first clocks special.
    // Later we can generalize this.
    bool isFirstClock = true;
};

template <typename T>
inline void OscSmoother2<T>::_primeForTest(float last)
{
    edgeDetector._primeForTest(last);
}

template <typename T>
inline bool OscSmoother2<T>::isLocked() const {
    return locked;
}

template <typename T>
inline T OscSmoother2<T>::_getPhaseInc() const
{
   return vco.getFrequency();
}

template <typename T>
inline float OscSmoother2<T>::step(float input) {
#ifdef _OSL
    printf("\nstep(%f)\n", input);
#endif
    // run the edge detector, look for low to high edge
    auto edge = edgeDetector.step(input);
    const bool newEdge = edge.first;
    const float phaseLag = edge.second;

    if (newEdge && isFirstClock) {
        isFirstClock = false;
        fractionalSamplesSinceReset = phaseLag;
#ifdef _OSL
        printf("swallowing first clock, frac = %f\n", fractionalSamplesSinceReset);
#endif
        return input;
    }

    if (newEdge) {
        integerPeriodsSinceReset++;
    }

  //  ++samplesSinceReset; 
#ifdef _OSL
    printf("after: edge = %d + %f, samples=%d per=%d\n", newEdge, edge.second, integerSamplesSinceReset, integerPeriodsSinceReset); fflush(stdout); 
    printf(" will comparse per since reset = %d with target %d\n", integerPeriodsSinceReset, smootherPeriodCycles);
#endif

    // now that we are ignoring first, maybe this more sensible >= compare will work?
    if (integerPeriodsSinceReset >= smootherPeriodCycles) {
        locked = true;

#ifdef _OSL
        printf("about to capture, int samples=%d, frac=%f, (sub) current lag = %f\n", integerSamplesSinceReset, fractionalSamplesSinceReset, phaseLag);
#endif

        // add one to make sample count come out right
        const T fullPeriodSampled = 1 + integerSamplesSinceReset + fractionalSamplesSinceReset - phaseLag;
        const T samplesPerCycle = fullPeriodSampled / T(smootherPeriodCycles);
       // const float samplesPerCycle = float(samplesSinceReset -1) / float(PERIOD_CYCLES);

#ifdef _OSL
        printf("*** captured %f samples per cycle %d per period\n", samplesPerCycle, integerSamplesSinceReset); fflush(stdout);
        printf("*** integer samples was %d, fract %f smootherPeriod=%d\n", integerSamplesSinceReset, fractionalSamplesSinceReset, smootherPeriodCycles);
#endif
        //    printf("or, using minus one %f\n", float(samplesSinceReset-1) / 16.f);

        // experiment - let's try constant
        //  const float newPhaseInc = 1.0f /  40.f;
        const T newPhaseInc = 1.0 / samplesPerCycle;

#ifdef _FULL_RESET
        vco.reset(newPhaseInc, phaseLag * newPhaseInc);
#else
        vco.setPitch(newPhaseInc);
#endif

        fractionalSamplesSinceReset = phaseLag;
        integerSamplesSinceReset = 0;
        integerPeriodsSinceReset = 0;
    } else {
        integerSamplesSinceReset++;
#ifdef _OSL
        printf("dindn't capture, not time yet\n");
#endif
    }
    vco.process();
    return float(10 * vco.getTriangle());
}
