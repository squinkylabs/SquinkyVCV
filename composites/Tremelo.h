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

   /* from vst
   		ControlValues() {
			lfoRate=3;		
			lfoShape=.5;
			lfoSkew=0;
			modDepth=0;
			lfoPhase=0;
			beatSync=0;
		}
        */
    enum ParamIds
    {
        LFO_RATE_PARAM,
        LFO_SHAPE_PARAM,
        LFO_SKEW_PARAM,
        MOD_DEPTH_PARAM,
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
