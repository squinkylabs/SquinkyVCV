#pragma once

#include "ClockMult.h"
#include "ObjectCache.h"


/*

old plug proc loop.

// Step 1: generate a saw
// range is 0..1
SawOsc<vec_t>::gen_v(*sawState, *sawParams, tempBuffer, sampleFrames);

// step 2: apply skew and phase shift
// range still 0..1
AsymRampShaper<vec_t>::proc_v(*shaperParams, tempBuffer, tempBuffer, sampleFrames);

// step 3: shift down to be centered at zero,
// max excursion +-5 at shape "most square"
// min is +/- .25  TODO: put the .25 into the control range itself

// range = +/- (5 * shape)
//
f_t shapeMul = std::max(.25, 10 * controlValues.lfoShape);
VecBasic<vec_t>::add_mul_c_imp(tempBuffer, sampleFrames, shapeMul, -.5f);

// now tanh,
// output contered around zero,
// max is tanh(.25) to tanh(5), depending on shape value
// rang = +/- tanh(5 * shape)
LookupUniform<vec_t>::lookup_clip_v(*tanhParams, tempBuffer, tempBuffer, sampleFrames);
*/

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
    void init();


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

    ClockMult clock;
    std::shared_ptr<LookupTableParams<float>> sinLookup;
  //  getSinLookup();
    float reciprocalSampleRate = 0;;
};



template <class TBase>
inline void Tremelo<TBase>::init()
{
    sinLookup = ObjectCache<float>::getSinLookup();
    clock.setDivisor(0);
}

static float mx = -10;
static float mn = 10;
static int ct = 0;

template <class TBase>
inline void Tremelo<TBase>::step()
{

    float r = TBase::params[LFO_RATE_PARAM].value;
    // scale crudely, for now.
    r += 5;
    r *= .2;
    r += .1;
    clock.setFreeRunFreq(r * reciprocalSampleRate);
    clock.sampleClock();

    float mod = clock.getSaw();
    mod = LookupTable<float>::lookup(*sinLookup.get(), mod);

 
    mn = std::min(mod, mn);
    mx = std::max(mod, mx);

    if (ct++ > 1000) {
        printf("mod = %f, %f\n", mn, mx);
        ct = 0;
    }

    TBase::outputs[AUDIO_OUTPUT].value = mod;   // just for now
}
