#pragma once

#include "AudioMath.h"
#include <assert.h>

template <typename T> class StateVariableFilterState2;
template <typename T> class StateVariableFilterParams2;

/**
 *
 *                        |-----------------------------------------------------> hi pass
 *                        |               |->Band                         (+)----->Notch
 *                        |               |                                |
 * input ->(  +    )------|---> Fc >--(+)-|-> Z**-1 >-|-> Fc >->(+)------>-.--|--> LowPass
 *          |(-1) | (-1)               |              |          |         |  |
 *          |     |                    |-<--------<---|          |-<Z**-1<-|  |
 *          |     |                                   |                       |
 *          |     -<-------------------------< Qc <----                       |
 *          |                                                                 |
 *			|---<-----------------------------------------------------------<--
 *
 *
 *
 *
 *
 *
 *
 */

template <typename T>
class StateVariableFilter2
{
public:
    enum class Mode
    {
        LowPass, BandPass, HighPass, Notch
    };
    StateVariableFilter2() = delete;       // we are only static
    typedef float_4 (*processFunction)(float_4 input, StateVariableFilterState2<T>& state, const StateVariableFilterParams2<T>& params);
    static processFunction getProcPointer(Mode);

    static T runLP(T input, StateVariableFilterState2<T>& state, const StateVariableFilterParams2<T>& params);
    static T runHP(T input, StateVariableFilterState2<T>& state, const StateVariableFilterParams2<T>& params);
    static T runBP(T input, StateVariableFilterState2<T>& state, const StateVariableFilterParams2<T>& params);
    static T runN(T input, StateVariableFilterState2<T>& state, const StateVariableFilterParams2<T>& params);
};

template <typename T>
inline  typename StateVariableFilter2<T>::processFunction StateVariableFilter2<T>::getProcPointer(Mode mode) {
    switch (mode) {
        case Mode::LowPass:
            return &runLP;
            break;
        case Mode::HighPass:
                return &runHP;
            break;
        case Mode::BandPass:
                return &runBP;
            break;
        case Mode::Notch:
                return &runN;
            break;
        default:
            assert(false);
    }
    return nullptr;
}

template <typename T>
inline T StateVariableFilter2<T>::runLP(T input, StateVariableFilterState2<T>& state, const StateVariableFilterParams2<T>& params)
{
    const T dLow = state.z2 + params.fcGain * state.z1;
    const T dHi = input - (state.z1 * params.qGain + dLow);
    const T dBand = dHi * params.fcGain + state.z1;  
    state.z1 = dBand;
    state.z2 = dLow;
    return dLow;
}

template <typename T>
inline T StateVariableFilter2<T>::runHP(T input, StateVariableFilterState2<T>& state, const StateVariableFilterParams2<T>& params)
{
    const T dLow = state.z2 + params.fcGain * state.z1;
    const T dHi = input - (state.z1 * params.qGain + dLow);
    const T dBand = dHi * params.fcGain + state.z1;  
    state.z1 = dBand;
    state.z2 = dLow;
    return dHi;
}

template <typename T>
inline T StateVariableFilter2<T>::runBP(T input, StateVariableFilterState2<T>& state, const StateVariableFilterParams2<T>& params)
{
    const T dLow = state.z2 + params.fcGain * state.z1;
    const T dHi = input - (state.z1 * params.qGain + dLow);
    const T dBand = dHi * params.fcGain + state.z1;  
    state.z1 = dBand;
    state.z2 = dLow;
    return dBand;
}

template <typename T>
inline T StateVariableFilter2<T>::runN(T input, StateVariableFilterState2<T>& state, const StateVariableFilterParams2<T>& params)
{
    const T dLow = state.z2 + params.fcGain * state.z1;
    const T dHi = input - (state.z1 * params.qGain + dLow);
    const T dBand = dHi * params.fcGain + state.z1;  
    state.z1 = dBand;
    state.z2 = dLow;
    return dLow + dHi;
}


/****************************************************************/

template <typename T>
class StateVariableFilterParams2
{
public:
    friend StateVariableFilter2<T>;

    /**
     * Set the filter Q.
     * Values must be > .5
     */
    void setQ(T q);

    /**
     * Normalized bandwidth is bw / fc
     * Also is 1 / Q
     */
    void setNormalizedBandwidth(T bw);
    T getNormalizedBandwidth() const
    {
        return qGain;
    }

    /**
     * Set the center frequency.
     * units are 1 == sample rate
     */
    void setFreq(T f);
    void setFreqAccurate(T f);

    #if 0
    void setMode(Mode m)
    {
        mode = m;
    }
    #endif

    T _fcGain() const { return fcGain; }
    T _qGain() const { return qGain; }
private:
    T qGain = 1.;		// internal amp gains
    T fcGain = T(.001);
};

template<>
inline void StateVariableFilterParams2<float>::setQ(float q)
{
    const float qLimit = .49;
    q = std::max(q, qLimit);
    qGain = 1 / q;
    //printf("q = %f, qg = %f\n", q, qGain);
}

template<>
inline void StateVariableFilterParams2<float_4>::setQ(float_4 q)
{
    const float_4 qLimit = .49;
    q = SimdBlocks::max(q, qLimit);
    qGain = 1 / q;
}

template <typename T>
inline void StateVariableFilterParams2<T>::setNormalizedBandwidth(T bw)
{
    qGain = bw;
}

template <>
inline void StateVariableFilterParams2<float>::setFreq(float fc)
{
    // Note that we are skipping the high freq warping.
    // Going for speed over accuracy
    fcGain = float(AudioMath::Pi) * 2.f * fc;
    fcGain = std::min(fcGain, .79f);

    //printf("two pole fc = %f, fcG = %f\n", fc, fcGain);
}

template <>
inline void StateVariableFilterParams2<float_4>::setFreq(float_4 fc)
{
    // Note that we are skipping the high freq warping.
    // Going for speed over accuracy
    fcGain = float_4(AudioMath::Pi) * float_4(2) * fc;
    fcGain = SimdBlocks::min(fcGain, float_4(.79));
}

template <typename T>
inline void StateVariableFilterParams2<T>::setFreqAccurate(T fc)
{
    assert(false);
   // fcGain = T(2) * std::sin( T(AudioMath::Pi) * fc);
}

/*******************************************************************************************/

template <typename T>
class StateVariableFilterState2
{
public:
    T z1 = 0;		// the delay line buffer
    T z2 = 0;		// the delay line buffer
};
