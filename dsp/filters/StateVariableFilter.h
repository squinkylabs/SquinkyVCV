#pragma once

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
    static T run(T input, StateVariableFilterState<T>& state, const StateVariableFilterParams<T>& params);
   // StateVariableStage();
   // double XFormOneSample(double);
#if 0
    enum class Mode
    {
        BandPass, LowPass, HiPass, Notch
    };
    void setQ(double q);
    void setFreq(double f);
    void setMode(Mode m)
    {
        mode = m;
    }
   // void SetFormat(const AudioFormat& f);
#endif

private:
#if 0
    Mode mode;

    double mdZ1;		// the delay line buffer
    double mdZ2;		// the delay line buffer

    double mdQGain;		// internal amp gains
    double mdFcGain;
    //
    double FreqToFcGain(double);
    //void SetTempFreq(double);
    static double QToQGain(double);

    AudioFormatHolder mFormat;
#endif
};

template <typename T> 
class StateVariableFilterParams
{
public:
    enum class Mode
    {
        BandPass, LowPass, HiPass, Notch
    };
    void setQ(double q);
    void setFreq(double f);
    void setMode(Mode m)
    {
        mode = m;
    }
private:
    Mode mode;
    T qGain;		// internal amp gains
    T fcGain;
};

template <typename T>
class StateVariableFilterState
{
public:
    double z1;		// the delay line buffer
    double z2;		// the delay line buffer
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


