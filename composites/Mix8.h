
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

private:

    Divider divider;
    float buf_inputs[numChannels];
    float buf_channelGains[numChannels];
    float buf_channelOuts[numChannels];
    float buf_leftPanGains[numChannels];
    float buf_rightPanGains[numChannels];
};


template <class TBase>
inline void Mix8<TBase>::init()
{
    const int divRate = 4;
    divider.setup(divRate, [this, divRate] {
        this->stepn(divRate);
        });
}

//#define _USEAVX
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


#else

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
#endif

template <class TBase>
inline void Mix8<TBase>::stepn(int div)
{
    // fill buf_channelGains
    for (int i = 0; i < numChannels; ++i) {
        buf_channelGains[i] = 1;
    }

    // fill buf_leftPanGains and buf_rightPanGains
    for (int i = 0; i < numChannels; ++i) {
        buf_leftPanGains[i] = 1;
        buf_rightPanGains[i] = 1;
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
            ret = {-5.0f, 5.0f, -5, "Level 1"};
            break;
        case Mix8<TBase>::GAIN1_PARAM:
            ret = {-5.0f, 5.0f, -5, "Level 2"};
            break;
        case Mix8<TBase>::GAIN2_PARAM:
            ret = {-5.0f, 5.0f, -5, "Level 3"};
            break;
        case Mix8<TBase>::GAIN3_PARAM:
            ret = {-5.0f, 5.0f, -5, "Level 4"};
            break;
        case Mix8<TBase>::GAIN4_PARAM:
            ret = {-5.0f, 5.0f, -5, "Level 5"};
            break;
        case Mix8<TBase>::GAIN5_PARAM:
            ret = {-5.0f, 5.0f, -5, "Level 6"};
            break;
        case Mix8<TBase>::GAIN6_PARAM:
            ret = {-5.0f, 5.0f, -5, "Level 7"};
            break;
        case Mix8<TBase>::GAIN7_PARAM:
            ret = {-5.0f, 5.0f, -5, "Level 8"};
            break;
        default:
            assert(false);
    }
    return ret;
}


