#pragma once

#include "AudioMath.h"
#include "FractionalDelay.h"
#include "ObjectCache.h"

template <class TBase>
class DaveguideDescription : public IComposite {
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Daveguide : public TBase {
public:
    Daveguide(Module* module) : TBase(module), delay(44100) {
        // init();
    }
    Daveguide() : TBase(), delay(44100) {
        // init();
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription() {
        return std::make_shared<DaveguideDescription<TBase>>();
    }

    enum ParamIds {
        OCTAVE_PARAM,
        TUNE_PARAM,
        DECAY_PARAM,
        FC_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
        AUDIO_INPUT,
        CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };

    /**
    * Main processing entry point. Called every sample
    */
    void step() override;

    float _freq = 0;

private:
    RecirculatingFractionalDelay delay;

    //static std::function<double(double)> makeFunc_Exp(double xMin, double xMax, double yMin, double yMax);

    // std::function<double(double)> delayScale = AudioMath::makeFunc_Exp(-5, 5, 1, 500);

    //  AudioMath::ScaleFun<float> feedbackScale = AudioMath::makeLinearScaler(0.f, 1.f);

    std::function<float(float)> expLookup = ObjectCache<float>::getExp2Ex();
};

template <class TBase>
void Daveguide<TBase>::step() {
#if 0
    // make delay knob to from 1 ms. to 1000
    double delayMS = delayScale(TBase::params[PARAM_DELAY].value);
    double feedback = feedbackScale(0, (TBase::params[PARAM_FEEDBACK].value), 1);

    double delaySeconds = delayMS * .001;
    double delaySamples = delaySeconds * TBase::engineGetSampleRate();

    delay.setDelay((float) delaySamples);
    delay.setFeedback((float) feedback);

    const float input = TBase::inputs[INPUT_AUDIO].value;
    const float output = delay.run(input);
    TBase::outputs[OUTPUT_AUDIO].value = output;
#endif
    float pitch = 1.0f + roundf(TBase::params[OCTAVE_PARAM].value) + TBase::params[TUNE_PARAM].value / 12.0f;
    pitch += TBase::inputs[CV_INPUT].getVoltage(0);

    const float q = float(log2(261.626));  // move up to pitch range of even vco
    pitch += q;
    _freq = expLookup(pitch);
    const float delaySeconds = 1.0f / _freq;
    float delaySamples = delaySeconds * TBase::engineGetSampleRate();

    delay.setDelay(delaySamples);
    // .999 -> 48 db peak to trough
    // .99  ->36 db
    // .9 = 24 db
    // .9999 -> 48
    delay.setFeedback(.9999f);

    const float input = TBase::inputs[AUDIO_INPUT].getVoltage(0);
    const float output = delay.run(input);
    TBase::outputs[AUDIO_OUTPUT].setVoltage(output, 0);
}

template <class TBase>
int DaveguideDescription<TBase>::getNumParams() {
    return Daveguide<TBase>::NUM_PARAMS;
}

/*
  OCTAVE_PARAM,
        TUNE_PARAM,
        DECAY_PARAM,
        FC_PARAM,
*/
template <class TBase>
inline IComposite::Config DaveguideDescription<TBase>::getParam(int i) {
    Config ret(0, 1, 0, "");
    switch (i) {
        case Daveguide<TBase>::OCTAVE_PARAM:
            ret = {-5, 5, 0, "Octave"};
            break;
        case Daveguide<TBase>::TUNE_PARAM:
            ret = {-5, 5, 0, "Tune"};
            break;
        case Daveguide<TBase>::DECAY_PARAM:
            ret = {-5, 5, 0, "Decay"};
            break;
        case Daveguide<TBase>::FC_PARAM:
            ret = {-5, 5, 0, "Fc"};
            break;
        default:
            assert(false);
    }
    return ret;
}