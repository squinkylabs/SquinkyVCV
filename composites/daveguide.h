#pragma once

#include "AudioMath.h"
#include "FractionalDelay.h"

template <class TBase>
class Daveguide : public TBase
{
public:
    Daveguide(struct Module * module) : TBase(module), delay(44100)
    {
       // init();
    }
    Daveguide() : TBase(), delay(44100)
    {
       // init();
    }

    enum ParamIds
    {
        PARAM_FEEDBACK,
        PARAM_DELAY,
        NUM_PARAMS
    };

    enum InputIds
    {
        INPUT_AUDIO,
        NUM_INPUTS
    };

    enum OutputIds
    {
        OUTPUT_AUDIO,
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
    RecirculatingFractionalDelay delay;

    //static std::function<double(double)> makeFunc_Exp(double xMin, double xMax, double yMin, double yMax);
    
    std::function<double(double)> delayScale = AudioMath::makeFunc_Exp(-5, 5, 1, 500);

    AudioMath::ScaleFun<float> feedbackScale = AudioMath::makeLinearScaler(0.f, 1.f);


};


template <class TBase>
void  Daveguide<TBase>::step()
{
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



    // clock.setMultiplier(1); // no mult
}