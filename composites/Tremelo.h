#pragma once


/**
 */
template <class TBase>
class Tremelo : public TBase
{
public:
    Tremelo(struct Module * module) : TBase(module)
    {
    }
    Tremelo() : TBase()
    {
    }
    void setSampleRate(float rate)
    {
        reciprocalSampleRate = 1 / rate;
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
        AUDIO_INPUT,
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

    /**
     * Main processing entry point. Called every sample
     */
    void step();


private:

    float reciprocalSampleRate;
};

template <class TBase>
inline void Tremelo<TBase>::step()
{
}
