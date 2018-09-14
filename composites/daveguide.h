#pragma once


template <class TBase>
class Daveguide : public TBase
{
public:
    Daveguide(struct Module * module) : TBase(module)
    {
       // init();
    }
    Daveguide() : TBase()
    {
       // init();
    }

    enum ParamIds
    {
        NUM_PARAMS
    };

    enum InputIds
    {
        NUM_INPUTS
    };

    enum OutputIds
    {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };
};