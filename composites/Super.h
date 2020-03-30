
#pragma once

#include "SuperDsp.h"
#include "Divider.h"
#include "IComposite.h"
//#include "ObjectCache.h"

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;

template <class TBase>
class SuperDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 * classic cpu: 14.4
 * clean: 29.6
 * clean2: 85.3
 */
template <class TBase>
class Super : public TBase
{
public:

    Super(Module* module) : TBase(module)
    {
        init();
    }
    Super() : TBase()
    {
        init();
    }

    /**
    * re-calculate everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<SuperDescription<TBase>>();
    }

    enum ParamIds
    {
        OCTAVE_PARAM,
        SEMI_PARAM,
        FINE_PARAM,
        DETUNE_PARAM,
        DETUNE_TRIM_PARAM,
        MIX_PARAM,
        MIX_TRIM_PARAM,
        FM_PARAM,
        CLEAN_PARAM,
        HARD_PAN_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CV_INPUT,
        TRIGGER_INPUT,
        DETUNE_INPUT,
        MIX_INPUT,
        FM_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MAIN_OUTPUT_LEFT,
        MAIN_OUTPUT_RIGHT,
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

    /** 
     * just for testing
     */
    SuperDsp& _getDsp(int n) {
        return dspCommon._getDsp(n);
    }

private:
    bool isStereo = false;
    Divider div;
    void updateStereo();
    void stepn(int);

    int getOversampleRate();
    const static int inputSubSample = 4;    // only look at knob/cv every 4

    SuperDspCommon dspCommon;
};

template <class TBase>
inline void Super<TBase>::init()
{
    div.setup(inputSubSample, [this] {
        this->stepn(div.getDiv());
     });

    // dspCommon.init();

    const int rate = getOversampleRate();
    const int decimateDiv = std::max(rate, (int) SuperDspCommon::MAX_OVERSAMPLE);

    dspCommon.setupDecimationRatio(decimateDiv, 16);
}

template <class TBase>
inline int Super<TBase>::getOversampleRate()
{
    int rate = 1;
    const int setting = (int) std::round(TBase::params[CLEAN_PARAM].value);
    switch (setting) {
        case 0:
            rate = 1;
            break;
        case 1:
            rate = 4;
            break;
        case 2:
            rate = 16;
            break;
        default:
            assert(false);
    }
    assert(rate <= (int)SuperDspCommon::MAX_OVERSAMPLE);
    return rate;
}

template <class TBase>
inline void Super<TBase>::updateStereo()
{
    isStereo = TBase::outputs[MAIN_OUTPUT_RIGHT].isConnected() && TBase::outputs[MAIN_OUTPUT_LEFT].isConnected(); 
}

template <class TBase>
inline void Super<TBase>::stepn(int n)
{
    updateStereo();

    int oversampleRate = getOversampleRate();
    float sampleTime = TBase::engineGetSampleTime();

    // The params
    float fineTuneParam =  TBase::params[FINE_PARAM].value;
    float semiParam =  TBase::params[SEMI_PARAM].value;
    float octaveParam =  TBase::params[OCTAVE_PARAM].value;
    float fmParam = TBase::params[FM_PARAM].value;
    float detuneParam = TBase::params[DETUNE_PARAM].value;
    float detuneTrimParam = TBase::params[DETUNE_TRIM_PARAM].value;
    float mixParam = TBase::params[MIX_PARAM].value;
    float mixTrimParam = TBase::params[MIX_TRIM_PARAM].value;
    const bool hardPan = TBase::params[HARD_PAN_PARAM].value > .5;

    const int numChannels = std::max<int>(1, TBase::inputs[CV_INPUT].channels);
    for (int i=0; i< numChannels; ++i) {
        dspCommon.stepn(n, i,  oversampleRate, sampleTime, TBase::inputs[CV_INPUT],
            fineTuneParam, semiParam, octaveParam, TBase::inputs[FM_INPUT],
            fmParam, TBase::inputs[DETUNE_INPUT], detuneParam, detuneTrimParam,
            TBase::inputs[MIX_INPUT], mixParam, mixTrimParam,
            isStereo,
            hardPan);
    }

    const int numOutputChannels = std::max(1, numChannels);

    TBase::outputs[MAIN_OUTPUT_LEFT].setChannels(numOutputChannels);
    TBase::outputs[MAIN_OUTPUT_RIGHT].setChannels(numOutputChannels);
}

template <class TBase>
inline void Super<TBase>::step()
{
    div.step();
    const int rate = getOversampleRate();

    // even unpatched we run 1 channel
    const int numChannels = std::max<int>(1, TBase::inputs[CV_INPUT].channels);
    dspCommon.step(numChannels,isStereo, TBase::outputs[MAIN_OUTPUT_LEFT], TBase::outputs[MAIN_OUTPUT_RIGHT],
        rate,
        TBase::inputs[TRIGGER_INPUT]);
}

template <class TBase>
int SuperDescription<TBase>::getNumParams()
{
    return Super<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config SuperDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Super<TBase>::OCTAVE_PARAM:
            ret = {-5, 4, 0, "Octave transpose"};
            break;
        case Super<TBase>::SEMI_PARAM:
            ret = {-11, 11, 0, "Semitone transpose"};
            break;
        case Super<TBase>::FINE_PARAM:
            ret = {-1, 1, 0, "Fine tune"};
            break;
        case Super<TBase>::DETUNE_PARAM:
            ret = {-5, 5, 0, "Detune"};
            break;
        case Super<TBase>::DETUNE_TRIM_PARAM:
            ret = {-1, 1, 0, "Detune CV trim"};
            break;
        case Super<TBase>::MIX_PARAM:
            ret = {-5, 5, 0, "Detuned saw level"};
            break;
        case Super<TBase>::MIX_TRIM_PARAM:
            ret = {-1, 1, 0, "Detuned saw CV trim"};
            break;
        case Super<TBase>::FM_PARAM:
            ret = {0, 1, 0, "Pitch modulation depth"};
            break;
        case Super<TBase>::CLEAN_PARAM:
            ret = {0.0f, 2, 0, "Alias suppression amount"};
            break;
        case Super<TBase>::HARD_PAN_PARAM:
            ret =  {0.0f, 1.0f, 0.0f, "Hard Pan"};
            break;
        default:
            assert(false);
    }
    return ret;
}

