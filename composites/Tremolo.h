
#pragma once

#include <vector>

#include "ClockMult.h"
#include "ObjectCache.h"
#include "AsymRampShaper.h"



class Stats
{
public:
    Stats(std::string s1, std::string s2, std::string s3)
    {
        labels.push_back(s1);
        labels.push_back(s2);
        labels.push_back(s3);
        data.resize(3);
        sum.resize(3);
        for (int i=0; i<3; ++i) {
            data[i].first = 100;
            data[i].second = -100;
            sum[i] = 0;
        }
    }
    void log1(float x) { log(0, x); }
    void log2(float x) { log(1, x); }
    void log3(float x) { log(2, x); }
private:
    std::vector<std::pair<float, float>> data;
    std::vector<std::string> labels;
    std::vector<double> sum;
    int dump_ct=0;
    int total_ct=0;

    void log(int index, float x)
    {
        #if 0
        data[index].first = std::min(data[index].first, x);
        data[index].second = std::max(data[index].second, x);
        if (++dump_ct > 1000) {
            dump();
            dump_ct = 0;
        }
        sum[index] += x;
        if (index == 0) {
            ++total_ct;
        }
        #endif
    }

    void dump()
    {
        printf("\n");
        for (int i=0; i<3; ++i) {
            printf("%s: %f, %f avg=%f\n",
                labels[i].c_str(),
                data[i].first,
                data[i].second,
                sum[i] / total_ct);
        }
        fflush(stdout);
    }

};

/**
 */
template <class TBase>
class Tremolo : public TBase
{
public:
    Tremolo(struct Module * module) : TBase(module), stats("saw", "mid", "final" )
    {
    }
    Tremolo() : TBase(), stats("saw", "mid", "final")
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
        CLOCK_MULT_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        CLOCK_INPUT,
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

    Stats stats;
    bool lastClock = false; // TODO: input conditioning
};



template <class TBase>
inline void Tremolo<TBase>::init()
{
    tanhLookup = ObjectCache<float>::getTanh5();
    clock.setMultiplier(0);

    scale_rate = AudioMath::makeBipolarAudioScaler(.1f, 10.f); // full CV range -> 0..1
    scale_skew = AudioMath::makeBipolarAudioScaler(-.99f, .99f);
    scale_shape = AudioMath::makeBipolarAudioScaler(0, 1);
    scale_depth = AudioMath::makeBipolarAudioScaler(0, 1);
    scale_phase = AudioMath::makeBipolarAudioScaler(-1, 1);
}

template <class TBase>
inline void Tremolo<TBase>::step()
{

    const bool clockIn = TBase::inputs[CLOCK_INPUT].value > 2;
    if (clockIn != lastClock) {
        lastClock = clockIn;
        if (clockIn) {
            clock.refClock();
        }
    }

    const int clockMul = (int)round(TBase::params[CLOCK_MULT_PARAM].value);
    clock.setMultiplier(clockMul);
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

    stats.log1(mod);

    // TODO: don't scale twice - just get it right the first tme
    const float shapeMul = std::max(.25f, 10 * shape);
    mod *= shapeMul;
    stats.log2(mod);

    mod = LookupTable<float>::lookup(*tanhLookup.get(), mod);
    TBase::outputs[LFO_OUTPUT].value = mod;   

    const float gain = modDepth / tanh(shapeMul/2);
  //  const float finalMul = gain + 1;    // TODO: offset control?
    const float finalMod = gain * mod + 1;      // TODO: this offset by 1 is pretty good, but we 
                                                // could add an offset control
    
    stats.log3(finalMod);
    TBase::outputs[AUDIO_OUTPUT].value = TBase::inputs[AUDIO_INPUT].value * finalMod;

// for test
   // TBase::outputs[AUDIO_OUTPUT].value = finalMod;

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

	// so: makeup gain of 1/tanh(shapeMul) will get us to +1/-1 
	// then multiply by depth to get contered around zero with correct depth
	// the add one to get back to trem range!
	f_t gain = controlValues.modDepth / tanh(shapeMul/2);
	VecBasic<vec_t>::mul_add_c_imp(tempBuffer, sampleFrames, gain, 1);
// scale then add constant
	// input = a * input + b
	static void mul_add_c_imp(f_t * inout, int size, f_t a, f_t b) {
		assert_size(size);

	// now range = +/- tanh(5*shape) * depth / tanh(10 * shape)
*/
