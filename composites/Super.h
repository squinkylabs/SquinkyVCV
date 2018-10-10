
#pragma once

#include "ObjectCache.h"


template <class TBase>
class Super : public TBase
{
public:

    Super(struct Module * module) : TBase(module)
    {
    }
    Super() : TBase()
    {
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
        OCTAVE_PARAM,
        SEMI_PARAM,
        FINE_PARAM,
        DETUNE_PARAM,
        MIX_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CV_INPUT,
        GATE_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        MAIN_OUTPUT,
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

private:
    static const int numSaws = 7;

    float phase[numSaws] = {0};
    float phaseInc[numSaws] =  {0};
    float globalPhaseInc = 0;

    std::function<float(float)> expLookup =
        ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaper =
        ObjectCache<float>::getAudioTaper();

    void updatePhaseInc();
    void updateAudio();

};


template <class TBase>
inline void Super<TBase>::init()
{
}

template <class TBase>
inline void Super<TBase>::updatePhaseInc()
{

    const float cv = TBase::inputs[CV_INPUT].value;
    
    const float finePitch = TBase::params[FINE_PARAM].value / 12.0f;
    const float semiPitch = TBase::params[SEMI_PARAM].value / 12.0f;


    float pitch = 1.0f + roundf(TBase::params[OCTAVE_PARAM].value) +
            semiPitch +
            finePitch;
    
    pitch += cv;

    const float q = float(log2(261.626));       // move up to pitch range of even vco
    pitch += q;
    const float freq = expLookup(pitch);
    globalPhaseInc = TBase::engineGetSampleTime() * freq;
}

template <class TBase>
inline void Super<TBase>::updateAudio()
{

}


template <class TBase>
inline void Super<TBase>::step()
{
    updatePhaseInc();
    updateAudio();
}

