#pragma once

template <class TBase>
class ColoredNoise : public TBase
{
public:
    ColoredNoise(struct Module * module) : TBase(module)
    {
    }
    ColoredNoise() : TBase()
    {
    }
    void setSampleRate(float rate)
    {
    }

    // must be called after setSampleRate
    void init()
    {
    }

    // Define all the enums here. This will let the tests and the widget access them.
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
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    /**
    * Main processing entry point. Called every sample
    */
    void step();

    typedef float T;        // use floats for all signals
};

template <class TBase>
void ColoredNoise<TBase>::step()
{

}