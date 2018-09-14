#pragma once

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

private:
    RecirculatingFractionalDelay delay;
};