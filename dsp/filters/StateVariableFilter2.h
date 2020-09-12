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
    StateVariableFilter() = delete;       // we are only static
    static T run(T input, StateVariableFilterState2<T>& state, const StateVariableFilterParams2<T>& params);

};

template <typename T>
inline T StateVariableFilter2<T>::run(T input, StateVariableFilterState2<T>& state, const StateVariableFilterParams2<T>& params)
{
    const T dLow = state.z2 + params.fcGain * state.z1;
    const T dHi = input - (state.z1 * params.qGain + dLow);
    T dBand = dHi * params.fcGain + state.z1;

    // TODO: figure out why we get these crazy values
#if 1
    if (dBand >= 1000) {
        dBand = 999;               // clip it
    }
    if (dBand < -1000) {
        dBand = -999;               // clip it
    }

#endif

    T d;
    switch (params.mode) {
        case StateVariableFilterParams2<T>::Mode::LowPass:
            d = dLow;
            break;
        case StateVariableFilterParams2<T>::Mode::HighPass:
            d = dHi;
            break;
        case StateVariableFilterParams2<T>::Mode::BandPass:
            d = dBand;
            break;
        case StateVariableFilterParams2<T>::Mode::Notch:
            d = dLow + dHi;
            break;
        default:
            assert(false);
            d = 0.0;
    }

    state.z1 = dBand;
    state.z2 = dLow;

    return d;
}

/****************************************************************/

template <typename T>
class StateVariableFilterParams2
{
public:
    friend StateVariableFilter2<T>;
    enum class Mode
    {
        BandPass, LowPass, HighPass, Notch
    };

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
    void setMode(Mode m)
    {
        mode = m;
    }
private:
    Mode mode = Mode::BandPass;
    T qGain = 1.;		// internal amp gains
    T fcGain = T(.001);
};

template <typename T>
inline void StateVariableFilterParams2<T>::setQ(T q)
{
    const float qLimit = .49;
    q = std::max(q, qLimit);
  
    qGain = 1 / q;
}

template <typename T>
inline void StateVariableFilterParams2<T>::setNormalizedBandwidth(T bw)
{
    qGain = bw;
}


template <typename T>
inline void StateVariableFilterParams2<T>::setFreq(T fc)
{
    const float fcLimit = .3;    // .3 is stable, .32 not
    fc = std::min(fcLimit, fc);
    // Note that we are skipping the high freq warping.
    // Going for speed over accuracy
    fcGain = T(AudioMath::Pi) * T(2) * fc;
}

template <typename T>
inline void StateVariableFilterParams2<T>::setFreqAccurate(T fc)
{
    fcGain = T(2) * std::sin( T(AudioMath::Pi) * fc);
}

/*******************************************************************************************/

template <typename T>
class StateVariableFilterState2
{
public:
    T z1 = 0;		// the delay line buffer
    T z2 = 0;		// the delay line buffer
};
