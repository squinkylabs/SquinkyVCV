
#pragma once

#include "Divider.h"
#include "IComposite.h"


#include <assert.h>
#include <immintrin.h>
#include <memory>


template <class TBase>
class Mix8Description : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};


/**
 * CPU usage, straight AS copy: 298
 *      reduce CV SR/4: 83.
 * 
 * Notes on how the AS mixer works.
 * VOL =  CH1_PARAM, 0.0f, 1.0f, 0.8f)
 * PAN = CH1_PAN_PARAM, -1.0f, 1.0f, 0.0f)
 * CH1MUTE , 0.0f, 1.0f, 0.0f
 * 
 * CH1_CV_INPUT
 * CH1_CV_PAN_INPUT
 * 
 * float ch1L =  
 *      (1-ch1m) * 
 *      (inputs[CH1_INPUT].value) *
 *      params[CH1_PARAM].value *
 *      PanL(   params[CH1_PAN_PARAM].value,
 *              (inputs[CH1_CV_PAN_INPUT].value))*
 *      clamp(  inputs[CH1_CV_INPUT].normalize(10.0f) / 10.0f,
 *              0.0f,
 *              1.0f);
 * 
 * so the mutes have no pop reduction
 * if (ch1mute.process(params[CH1MUTE].value)) {
		ch1m = !ch1m;
	}

float PanL(float balance, float cv) { // -1...+1
		float p, inp;
		inp = balance + cv / 5;
		p = M_PI * (clamp(inp, -1.0f, 1.0f) + 1) / 4;
		return ::cos(p);
	}

	float PanR(float balance , float cv) {
		float p, inp;
		inp = balance + cv / 5;
		p = M_PI * (clamp(inp, -1.0f, 1.0f) + 1) / 4;
		return ::sin(p);
	}

    so, in english, the gain is: sliderPos * panL(knob, cv) * clamped&scaled CV

    plan:
        make all the params have the same range.
        implement the channel volumes -> all the way to out.
        implement the pan, using slow math.
        make lookup tables.
        implement mute, with no pop

 */

template <class TBase>
class Mix8 : public TBase
{
public:

    Mix8(struct Module * module) : TBase(module)
    {
    }
    Mix8() : TBase()
    {
    }

    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    enum ParamIds
    {
        GAIN0_PARAM,
        GAIN1_PARAM,
        GAIN2_PARAM,
        GAIN3_PARAM,
        GAIN4_PARAM,
        GAIN5_PARAM,
        GAIN6_PARAM,
        GAIN7_PARAM,
        PAN0_PARAM,
        PAN1_PARAM,
        PAN2_PARAM,
        PAN3_PARAM,
        PAN4_PARAM,
        PAN5_PARAM,
        PAN6_PARAM,
        PAN7_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO0_INPUT,
        AUDIO1_INPUT,
        AUDIO2_INPUT,
        AUDIO3_INPUT,
        AUDIO4_INPUT,
        AUDIO5_INPUT,
        AUDIO6_INPUT,
        AUDIO7_INPUT,
        LEVEL0_INPUT,
        LEVEL1_INPUT,
        LEVEL2_INPUT,
        LEVEL3_INPUT,
        LEVEL4_INPUT,
        LEVEL5_INPUT,
        LEVEL6_INPUT,
        LEVEL7_INPUT,
        PAN0_INPUT,
        PAN1_INPUT,
        PAN2_INPUT,
        PAN3_INPUT,
        PAN4_INPUT,
        PAN5_INPUT,
        PAN6_INPUT,
        PAN7_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        CHANNEL0_OUTPUT,
        CHANNEL1_OUTPUT,
        CHANNEL2_OUTPUT,
        CHANNEL3_OUTPUT,
        CHANNEL4_OUTPUT,
        CHANNEL5_OUTPUT,
        CHANNEL6_OUTPUT,
        CHANNEL7_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<Mix8Description<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    void stepn(int steps);

    const static int numChannels = 8;

    float buf_inputs[numChannels];
    float buf_channelGains[numChannels];
    float buf_channelOuts[numChannels];
    float buf_leftPanGains[numChannels];
    float buf_rightPanGains[numChannels];

 private:
     Divider divider;
};

#ifndef _CLAMP
#define _CLAMP
namespace std {
    inline float clamp(float v, float lo, float hi)
    {
        assert(lo < hi);
        return std::min(hi, std::max(v, lo));
    }
}
#endif


static inline float PanL(float balance, float cv) { // -1...+1
    float p, inp;
    inp = balance + cv / 5;
    p = M_PI * (std::clamp(inp, -1.0f, 1.0f) + 1) / 4;
    return ::cos(p);
}

static inline float PanR(float balance , float cv) {
    float p, inp;
    inp = balance + cv / 5;
    p = M_PI * (std::clamp(inp, -1.0f, 1.0f) + 1) / 4;
    return ::sin(p);
}

template <class TBase>
inline void Mix8<TBase>::stepn(int div)
{
    // fill buf_channelGains
    for (int i = 0; i < numChannels; ++i) {
        const float slider = TBase::params[i + GAIN0_PARAM].value;

        // TODO: get rid of normalize. if active ? cv : 10;
        const float cv = std::clamp(  
            TBase::inputs[i + LEVEL0_INPUT].normalize(10.0f) / 10.0f,
            0.0f,
            1.0f);
        buf_channelGains[i] = slider * cv;
    }

    // fill buf_leftPanGains and buf_rightPanGains
    for (int i = 0; i < numChannels; ++i) {
        const float balance = TBase::params[i + PAN0_PARAM].value;
        const float cv = TBase::inputs[i + PAN0_INPUT].value;
        buf_leftPanGains[i] = PanL(balance, cv);

       // const float balanceR = TBase::params[i + PAN0_PARAM].value;
        //const float cvR = TBase::inputs[i PAN0_INPUT].value;
        buf_rightPanGains[i] = PanR(balance, cv);
    }
}


template <class TBase>
inline void Mix8<TBase>::init()
{
    const int divRate = 1;
    divider.setup(divRate, [this, divRate] {
        this->stepn(divRate);
        });
}

template <class TBase>
inline void Mix8<TBase>::step()
{
    divider.step();

    // fill buf_inputs
    for (int i = 0; i < numChannels; ++i) {
        buf_inputs[i] = TBase::inputs[i + AUDIO0_INPUT].value;
    }

    // compute buf_channelOuts
    for (int i = 0; i < numChannels; ++i) {
        buf_channelOuts[i] = buf_inputs[i] * buf_channelGains[i];
    }

    // compute and output master outputs
    float left = 0, right = 0;
    for (int i = 0; i < numChannels; ++i) {
        left += buf_channelOuts[i] * buf_leftPanGains[i];
        right += buf_channelOuts[i] * buf_rightPanGains[i];
    }

    TBase::outputs[LEFT_OUTPUT].value = left;
    TBase::outputs[RIGHT_OUTPUT].value = right;

    // output channel outputs
    for (int i = 0; i < numChannels; ++i) {
        TBase::outputs[i + CHANNEL0_OUTPUT].value = buf_channelOuts[i];
    }
}

template <class TBase>
int Mix8Description<TBase>::getNumParams()
{
    return Mix8<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config Mix8Description<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Mix8<TBase>::GAIN0_PARAM:
            ret = {0, 1, .8f, "Level 1"};
            break;
        case Mix8<TBase>::GAIN1_PARAM:
            ret = {0, 1, .8f, "Level 2"};
            break;
        case Mix8<TBase>::GAIN2_PARAM:
            ret = {0, 1, .8f, "Level 3"};
            break;
        case Mix8<TBase>::GAIN3_PARAM:
            ret = {0, 1, .8f, "Level 4"};
            break;
        case Mix8<TBase>::GAIN4_PARAM:
            ret = {0, 1, .8f, "Level 5"};
            break;
        case Mix8<TBase>::GAIN5_PARAM:
            ret = {0, 1, .8f, "Level 6"};
            break;
        case Mix8<TBase>::GAIN6_PARAM:
            ret = {0, 1, .8f, "Level 7"};
            break;
        case Mix8<TBase>::GAIN7_PARAM:
            ret = {0, 1, .8f, "Level 8"};
            break;
        case Mix8<TBase>::PAN0_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 1"};
            break;
        case Mix8<TBase>::PAN1_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 2"};
            break;
        case Mix8<TBase>::PAN2_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 3"};
            break;
        case Mix8<TBase>::PAN3_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 4"};
            break;
        case Mix8<TBase>::PAN4_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 5"};
            break;
        case Mix8<TBase>::PAN5_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 6"};
            break;
        case Mix8<TBase>::PAN6_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 7"};
            break;
        case Mix8<TBase>::PAN7_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 8"};
            break;

        default:
            assert(false);
    }
    return ret;
}

#ifdef _USEAVX


template <class TBase>
inline void Mix8<TBase>::step()
{
    divider.step();

    // fill buf_inputs
    for (int i = 0; i < numChannels; ++i) {
        buf_inputs[i] = TBase::inputs[i + AUDIO0_INPUT].value;
    }

     // compute buf_channelOuts
    __m256 inputs = _mm256_load_ps(buf_inputs);
    __m256 gains = _mm256_load_ps(buf_channelGains);
    inputs = _mm256_mul_ps(inputs, gains);
    _mm256_store_ps(buf_channelOuts, inputs);

    // inputs now is a vector of channel outputs
    __m256 leftGains = _mm256_load_ps(buf_leftPanGains);
    __m256 rightGains = _mm256_load_ps(buf_rightPanGains);
    __m256 left = _mm256_mul_ps(inputs, leftGains);
    __m256 right = _mm256_mul_ps(inputs, rightGains);



    __m256 zero = _mm256_set1_ps(0);
    left = _mm256_hadd_ps(left, zero);
    left = _mm256_hadd_ps(left, zero);
    left = _mm256_hadd_ps(left, zero);

    right = _mm256_hadd_ps(right, zero);
    right = _mm256_hadd_ps(right, zero);
    right = _mm256_hadd_ps(right, zero);

    float temp[8];
    _mm256_store_ps(temp, right);
    TBase::outputs[RIGHT_OUTPUT].value = temp[0];
    _mm256_store_ps(temp, left);
    TBase::outputs[LEFT_OUTPUT].value = temp[0];

      // output channel outputs
    for (int i = 0; i < numChannels; ++i) {
        TBase::outputs[i + CHANNEL0_OUTPUT].value = buf_channelOuts[i];
    }

}

#endif


