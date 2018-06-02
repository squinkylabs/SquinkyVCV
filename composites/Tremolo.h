#pragma once

#include "ClockMult.h"
#include "ObjectCache.h"
#include "AsymRampShaper.h"

/*


ControlValues() {
lfoRate=3;
lfoShape=.5;
lfoSkew=0;
modDepth=0;
lfoPhase=0;
beatSync=0;
}

{
IEffectParameter * p = new LogParam("freq", "hz",
.1, 10, 3);
effectParameterList.params.push_back(p);
}
{
IEffectParameter * p = new LinearParam("shape", " ",
0, 1, 0);
effectParameterList.params.push_back(p);
}
{
IEffectParameter * p = new LinearParam("skew", " ",
-.99, .99, 0);
effectParameterList.params.push_back(p);
}
{
IEffectParameter * p = new LinearParam("phase", " ",
-1, 1, 0);
effectParameterList.params.push_back(p);
}
{
IEffectParameter * p = new LinearParam("depth", " ",
0, 1, 0);
effectParameterList.params.push_back(p);
}
{
//IEffectParameter * p = new ComboBoxParam("depth", " ",
//	0, 1, 0);
std::vector< std::string > values;
for (int i=0; bsettings[i].label; ++i) {
values.push_back( bsettings[i].label);
}
IEffectParameter * p = new ComboBoxParam("Beat", values, 0);
effectParameterList.params.push_back(p);
}
}

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
class Tremolo : public TBase
{
public:
    Tremolo(struct Module * module) : TBase(module)
    {
    }
    Tremolo() : TBase()
    {
    }
    void setSampleRate(float rate)
    {
        reciprocalSampleRate = 1 / rate;
    }

    // must be called after setSampleRate
    void init();

    enum ParamIds
    {
        LFO_RATE_PARAM,
        LFO_SHAPE_PARAM,
        LFO_SKEW_PARAM,
        LFO_PHASE_PARAM,
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
        SAW_OUTPUT,
        LFO_OUTPUT,
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
    std::shared_ptr<LookupTableParams<float>> tanhLookup;
    float reciprocalSampleRate = 0;

    AsymRampShaperParams rampShaper;

    // make some bootstrap scalers
    AudioMath::ScaleFun<float> scale_rate;
    AudioMath::ScaleFun<float> scale_skew;
    AudioMath::ScaleFun<float> scale_shape;
    AudioMath::ScaleFun<float> scale_depth;
    AudioMath::ScaleFun<float> scale_phase;
};



template <class TBase>
inline void Tremolo<TBase>::init()
{
    tanhLookup = ObjectCache<float>::getTanh5();
    clock.setDivisor(0);

    scale_rate = AudioMath::makeBipolarAudioScaler(.1f, 10.f); // full CV range -> 0..1
    scale_skew = AudioMath::makeBipolarAudioScaler(-.99f, .99f);
    scale_shape = AudioMath::makeBipolarAudioScaler(0, 1);
    scale_depth = AudioMath::makeBipolarAudioScaler(0, 1);
    scale_phase = AudioMath::makeBipolarAudioScaler(-1, 1);
}

template <class TBase>
inline void Tremolo<TBase>::step()
{
    // .1...10
    const float rate = scale_rate(0, 
        TBase::params[LFO_RATE_PARAM].value,
        1);
    const float shape = scale_shape(0,
        TBase::params[LFO_SHAPE_PARAM].value,
        1);

    const float skew = scale_skew(0,
        TBase::params[LFO_SKEW_PARAM].value,
        1);
    const float phase = scale_phase(0,
        TBase::params[LFO_PHASE_PARAM].value,
        1);
    const float modDepth = scale_depth(0,
        TBase::params[MOD_DEPTH_PARAM].value,
        1);
   

    clock.setFreeRunFreq(rate * reciprocalSampleRate);


    // For now, call setup every sample. will eat a lot of cpu
    AsymRampShaper::setup(rampShaper, skew, phase);



    // ------------ now generate the lfo waveform
    clock.sampleClock();
    float mod = clock.getSaw();
    mod = AsymRampShaper::proc_1(rampShaper, mod);
    mod -= 0.5f;
    // now we have a skewed saw -.5 to .5
    TBase::outputs[SAW_OUTPUT].value = mod;

    // TODO: don't scale twice - just get it right the first tme
    const float shapeMul = std::max(.25f, 10 * shape);
    mod *= shapeMul;

    mod = LookupTable<float>::lookup(*tanhLookup.get(), mod);


    TBase::outputs[LFO_OUTPUT].value = mod;   // just for now
}

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
