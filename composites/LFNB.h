
#pragma once

#include "ButterworthFilterDesigner.h"
#include "Decimator.h"
#include "LowpassFilter.h"
#include "BiquadParams.h"
#include "BiquadState.h"
#include "BiquadFilter.h"
#include "Divider.h"
#include "ObjectCache.h"
#include "IComposite.h"
#include "StateVariableFilter.h"
#include <random>

/**
 * Noise generator feeding a bandpass filter.
 * Calculated at very low sample rate, then re-sampled
 * up to audio rate.
 *  CV knob scaling: freq. working backwards:
 *      want the device to span .01 hz to 10 hz.
 *      so decimate by 100, have filter 1hz to 10k
 *      if filter wants exp sweep, let's map that exp like:
 *          fm = 4   k = 2;
 *          fm = 8   k = 3;        
 *          fm = 16k k = 14;         
 * 
 *  fc = fm / 4
 *  fm = exp2(k).
 *  k is linear scalar range, 2..14
 * 
 * For Q, let's start linear 1..50
 * 
 */

/** does the DSP processing
 *
 * TODO:
 *      get the bandpass working a audio rates.
 */
class LFNBChannel
{
public:
    void setSampleTime(float sampleTime)
    {
        float decimationDivisor = 100;          // only for Fc = 1;
        designLPF(sampleTime, decimationDivisor);
        setupDecimator(decimationDivisor);
    }

    /* with just noise, it's ok.
     * with decimated noise, it's ok (but blocky looking
     */
    float step()
    {
        bool needsData;
        TButter x = decimator.clock(needsData);
        x = BiquadFilter<TButter>::run(x, lpfState, lpfParams);
        if (needsData) {
            double z = 50 * StateVariableFilter<double>::run(noise(), bpState, bpParams);
            z /= sqrt(_fc);           // boost output at low freq. But this will over compensate! do we need log f here?
            z *= .007;
            decimator.acceptData( float(z));

        }
        return float(x);
    }

    void setFilter(float fc, float q)
    {
        bpParams.setFreq(fc);
        bpParams.setQ(q);
        _fc = fc;
    }
    LFNBChannel()
    {
        bpParams.setMode(StateVariableFilterParams<double>::Mode::BandPass);
    }

private:

    void designLPF(float sampleTime, float decimationDivisor)
    {
        const float lpFc = 50 * sampleTime;        // for now, let's try 100 hz. probably too high
        ButterworthFilterDesigner<TButter>::designThreePoleLowpass(
            lpfParams, lpFc);
    }
    void setupDecimator(float decimationDivisor)
    {
        decimator.setDecimationRate(decimationDivisor);
    }
    ::Decimator decimator;

    // the bandpass filter
    StateVariableFilterState<double> bpState;
    StateVariableFilterParams<double> bpParams;

    /**
     * Template type for butterworth reconstruction filter
     * Tried double for best low frequency performance. It's
     * probably overkill, but calculates plenty fast.
     */
    using TButter = double;
    BiquadParams<TButter, 2> lpfParams;
    BiquadState<TButter, 2> lpfState;

    std::default_random_engine generator{57};
    std::normal_distribution<double> distribution{-1.0, 1.0};
    float noise()
    {
        return  (float) distribution(generator);
    }
    float _fc = .1f;
};


template <class TBase>
class LFNBDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class LFNB : public TBase
{
public:

    LFNB(Module * module) : TBase(module)
    {
    }
    LFNB() : TBase()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<LFNBDescription<TBase>>();
    }

    void onSampleRateChange()
    {
        const float s = this->engineGetSampleTime();
        channels[0].setSampleTime(s);
        channels[1].setSampleTime(s);
    }

    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    enum ParamIds
    {
        FC0_PARAM,
        FC1_PARAM,
        Q0_PARAM,
        Q1_PARAM,
        FC0_TRIM_PARAM,
        FC1_TRIM_PARAM,
        Q0_TRIM_PARAM,
        Q1_TRIM_PARAM,
        XLFNB_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        FC0_INPUT,
        FC1_INPUT,
        Q0_INPUT,
        Q1_INPUT,
     //   AUDIO0_INPUT,
     //   AUDIO1_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        AUDIO0_OUTPUT,
        AUDIO1_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    float getBaseFrequency() const
    {
        return baseFrequency;
    }

    bool isXLFN() const
    {
        return  TBase::params[XLFNB_PARAM].value > .5;
    }

    /**
     * This lets the butterworth get re-calculated on the UI thread.
     * We can't do it on the audio thread, because it calls malloc.
     */
    void pollForChangeOnUIThread();

private:

    LFNBChannel channels[2];
    Divider divider;

    void stepn(int div);

    /**
     * Frequency, in Hz, of the lowest band in the graphic EQ
     */
    float baseFrequency = 1;

   /**
    * The last value baked by the LPF filter calculation
    * done on the UI thread.
    */
    float lastBaseFrequencyParamValue = -100;
    float lastXLFMParamValue = -1;

   // int controlUpdateCount = 0;

    /**
     * Must be called after baseFrequency is updated.
     * re-calculates the butterworth lowpass.
     */
  //  void updateLPF();

    /**
     * scaling function for the range / base frequency knob
     * map knob range from .1 Hz to 2.0 Hz
     */
    std::function<double(double)> rangeFunc =
    {AudioMath::makeFunc_Exp(-5, 5, .1, 2)};

/**
 * Audio taper for the EQ gains. Arbitrary max value selected
 * to give "good" output level.
 */
    AudioMath::SimpleScaleFun<float> gainScale =
    {AudioMath::makeSimpleScalerAudioTaper(0, 35)};


// new stuff
    std::shared_ptr<LookupTableParams<float>> expLookup = ObjectCache<float>::getExp2();
    AudioMath::ScaleFun<float> cvLinearScalar = AudioMath::makeLinearScaler(2.f, 14.f);
    AudioMath::ScaleFun<float> qLinearScalar  = AudioMath::makeLinearScaler(1.f, 30.f);
};


template <class TBase>
inline void LFNB<TBase>::pollForChangeOnUIThread()
{

}

template <class TBase>
inline void LFNB<TBase>::init()
{
    divider.setup(4, [this]() {
        stepn(4);
    });
}


template <class TBase>
inline void LFNB<TBase>::stepn(int)
{
    // update the BP filter base on fc,q knobs and cv
    float k = cvLinearScalar(
        TBase::inputs[FC0_INPUT].value,
        TBase::params[FC0_PARAM].value,
        TBase::params[FC0_TRIM_PARAM].value);

    float fm =  LookupTable<float>::lookup(*expLookup, k); 
    float fc = fm / 4;

   

    float q = qLinearScalar(
        TBase::inputs[Q0_INPUT].value,
        TBase::params[Q0_PARAM].value,
        TBase::params[Q0_TRIM_PARAM].value);

     channels[0].setFilter(fc * this->engineGetSampleTime(), q);
}

template <class TBase>
inline void LFNB<TBase>::step()
{
    divider.step();

    for (int i = 0; i < 2; ++i) {
        float x = channels[i].step();
        TBase::outputs[AUDIO0_OUTPUT+i].value = (float) x;
    }
#if 0
    // Let's only check the inputs every 4 samples. Still plenty fast, but
    // get the CPU usage down really far.
    if (controlUpdateCount++ > 4) {
        controlUpdateCount = 0;
        const int numEqStages = geq.getNumStages();
        for (int i = 0; i < numEqStages; ++i) {
            auto paramNum = i + EQ0_PARAM;
            auto cvNum = i + EQ0_INPUT;
            const float gainParamKnob = TBase::params[paramNum].value;
            const float gainParamCV = TBase::inputs[cvNum].value;
            const float gain = gainScale(gainParamKnob, gainParamCV);
            geq.setGain(i, gain);
        }
    }

    bool needsData;
    TButter x = decimator.clock(needsData);
    x = BiquadFilter<TButter>::run(x, lpfState, lpfParams);
    if (needsData) {
        const float z = geq.run(noise());
        decimator.acceptData(z);
    }

    TBase::outputs[OUTPUT].value = (float) x;
#endif
}



template <class TBase>
int LFNBDescription<TBase>::getNumParams()
{
    return LFNB<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config LFNBDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch(i) {
        case LFNB<TBase>::FC0_PARAM:
            ret = { -5, 5, 0, "Frequency 1"};
            break;
        case LFNB<TBase>::Q0_PARAM:
            ret = { -5, 5, 0, "Filter Q 1"};
            break;
        case LFNB<TBase>::FC0_TRIM_PARAM:
            ret = {-5, 5, 0, "Frequency CV trim 1"};
            break;
        case LFNB<TBase>::Q0_TRIM_PARAM:
            ret = {-5, 5, 0, "Filter Q CV trim 1"};
            break;
        case LFNB<TBase>::FC1_PARAM:
            ret = {-5, 5, 0, "Frequency 2"};
            break;
        case LFNB<TBase>::Q1_PARAM:
            ret = {-5, 5, 0, "Filter Q 2"};
            break;
        case LFNB<TBase>::FC1_TRIM_PARAM:
            ret = {-5, 5, 0, "Frequency CV trim 2"};
            break;
        case LFNB<TBase>::Q1_TRIM_PARAM:
            ret = {-5, 5, 0, "Filter Q CV trim 2"};
            break;
        case LFNB<TBase>::XLFNB_PARAM:
            ret = { 0, 1, 0, "Extra low frequency"};
            break;
        default:
            assert(false);
    }
    return ret;
}


