#pragma once

#include "AudioMath.h"

template <typename T> class StateVariableFilterState;
template <typename T> class StateVariableFilterParams;

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
class StateVariableFilter
{
public:
    StateVariableFilter() = delete;       // we are only static
    static T run(T input, StateVariableFilterState<T>& state, const StateVariableFilterParams<T>& params);

};

template <typename T>
inline T StateVariableFilter<T>::run(T input, StateVariableFilterState<T>& state, const StateVariableFilterParams<T>& params)
{
    const T dLow = state.z2 + params.fcGain * state.z1;
    const T dHi = input - (state.z1 * params.qGain + dLow);
    T dBand = dHi * params.fcGain + state.z1;

    // TODO: figure out why we get these crazy values
#if 1
    if (dBand >= 1000) {
       // fprintf(stderr, "dband = %f, dhi=%f fcG=%f z1=%f\n",
      //    dBand, dHi, params.fcGain, state.z1);
       // fflush(stderr);
        dBand = 999;               // clip it
    }
    if (dBand < -1000) {
       // fprintf(stderr, "dband = %f, dhi=%f fcG=%f z1=%f\n",
        //    dBand, dHi, params.fcGain, state.z1);
       // fflush(stderr);
        dBand = -999;               // clip it
    }

#endif
   // assert(dBand < 1000.0);
   // assert(dBand > -1000.0);
    T d;
    switch (params.mode) {
        case StateVariableFilterParams<T>::Mode::LowPass:
            d = dLow;
            break;
        case StateVariableFilterParams<T>::Mode::HiPass:
            d = dHi;
            break;
        case StateVariableFilterParams<T>::Mode::BandPass:
            d = dBand;
            break;
        case StateVariableFilterParams<T>::Mode::Notch:
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
class StateVariableFilterParams
{
public:
    friend StateVariableFilter<T>;
    enum class Mode
    {
        BandPass, LowPass, HiPass, Notch
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

    /**
     * Set the center frequency.
     * units are 1 == sample rate
     */
    void setFreq(T f);
    void setMode(Mode m)
    {
        mode = m;
    }
private:
    Mode mode = Mode::BandPass;
    T qGain= 1.;		// internal amp gains
    T fcGain= T(.001);
};

template <typename T>
inline void StateVariableFilterParams<T>::setQ(T q)
{
    if (q < .49) {
        assert(false);
        q = T(.6);
    }
    qGain = 1 / q;
}


template <typename T>
inline void StateVariableFilterParams<T>::setNormalizedBandwidth(T bw)
{
    qGain = bw;
}

template <typename T>
inline void StateVariableFilterParams<T>::setFreq(T fc)
{
    // Note that we are skipping the high freq warping.
    // Going for speed over accuracy
    fcGain =  T(AudioMath::Pi) * T(2) * fc;
}

/*******************************************************************************************/

template <typename T>
class StateVariableFilterState
{
public:
    T z1=0;		// the delay line buffer
    T z2=0;		// the delay line buffer
};


/*
#if 0
inline double StateVariableStage::XFormOneSample(double dIn)
{
    const double dLow = mdZ2 + mdFcGain * mdZ1;
    const double dHi = dIn - (mdZ1 * mdQGain + dLow);
    const double dBand = dHi * mdFcGain + mdZ1;

    ASSERT(dBand < 1000.0);

    double d;
    switch (meMode) {
        case LowPass:
            d = dLow;
            break;
        case HiPass:
            d = dHi;
            break;
        case BandPass:
            d = dBand;
            break;
        case Notch:
            d = dLow + dHi;
            break;
        default:
            ASSERT(0);
            d = 0.0;
    }

    mdZ1 = dBand;
    mdZ2 = dLow;
    return d;
}

#endif

*/


