
#pragma once

#include "Divider.h"
#include "IComposite.h"
#include "MultiLag.h"
#include "ObjectCache.h"
#include "SqMath.h"


#include <assert.h>
#include <immintrin.h>
#include <memory>


template <class TBase>
class Mix4Description : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
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

/**
 Perf: 10.4 before new features

 */

template <class TBase>
class Mix4 : public TBase
{
public:
    Mix4(Module * module) : TBase(module)
    {
    }
    Mix4() : TBase()
    {
    }
    static const int numChannels = 4;

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
       
        PAN0_PARAM,
        PAN1_PARAM,
        PAN2_PARAM,
        PAN3_PARAM,
        
        MUTE0_PARAM,
        MUTE1_PARAM,
        MUTE2_PARAM,
        MUTE3_PARAM,
      
        SOLO0_PARAM,
        SOLO1_PARAM,
        SOLO2_PARAM,
        SOLO3_PARAM,

        SEND0_PARAM,
        SEND1_PARAM,
        SEND2_PARAM,
        SEND3_PARAM,
       
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO0_INPUT,
        AUDIO1_INPUT,
        AUDIO2_INPUT,
        AUDIO3_INPUT,
      
        LEVEL0_INPUT,
        LEVEL1_INPUT,
        LEVEL2_INPUT,
        LEVEL3_INPUT,
      
        PAN0_INPUT,
        PAN1_INPUT,
        PAN2_INPUT,
        PAN3_INPUT,

        MUTE0_INPUT,
        MUTE1_INPUT,
        MUTE2_INPUT,
        MUTE3_INPUT,

        NUM_INPUTS
    };

    enum OutputIds
    {
        CHANNEL0_OUTPUT,
        CHANNEL1_OUTPUT,
        CHANNEL2_OUTPUT,
        CHANNEL3_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        SOLO0_LIGHT,
        SOLO1_LIGHT,
        SOLO2_LIGHT,
        SOLO3_LIGHT,
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<Mix4Description<TBase>>();
    }

    void setExpansionInputs(const float*);
    void setExpansionOutputs(float*);
    void requestModuleSolo(int channel);

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    void stepn(int steps);

    float buf_inputs[numChannels];
    float buf_channelGains[numChannels];
    float buf_channelSendGains[numChannels];
    float buf_channelOuts[numChannels];
    float buf_leftPanGains[numChannels];
    float buf_rightPanGains[numChannels];


    float buf_muteInputs[numChannels];
private:
    Divider divider;

    MultiLPF<numChannels> antiPop;
    std::shared_ptr<LookupTableParams<float>> panL = ObjectCache<float>::getMixerPanL();
    std::shared_ptr<LookupTableParams<float>> panR = ObjectCache<float>::getMixerPanR();

    const float* expansionInputs = nullptr;
    float* expansionOutputs = nullptr;

    /**
     * 0 = no solo
     * 1..4 = soloe 0..3
     */
    int     soloChannel=0;

};

// called from audio thread
template <class TBase>
inline  void Mix4<TBase>::requestModuleSolo(int channel)
{
    soloChannel = channel;
    printf("Mix4::requestModuleSolo(%d)\n", channel);
    int ch = soloChannel-1;
    for (int i=0; i<4; ++i) {
        TBase::lights[i + SOLO0_LIGHT].value = (ch == i) ? 10.f : 0.f;
        printf("light[%d] = %f\n", i, (ch == i) ? 10.f : 0.f);
    }
}


template <class TBase>
inline void Mix4<TBase>::stepn(int div)
{
    // fill buf_channelGains
    for (int i = 0; i < numChannels; ++i) {
        const float slider = TBase::params[i + GAIN0_PARAM].value;

        // TODO: get rid of normalize. if active ? cv : 10;
        const float rawCV = TBase::inputs[i + LEVEL0_INPUT].active ? 
             TBase::inputs[i + LEVEL0_INPUT].value : 10.f;
        const float cv = std::clamp(
            rawCV / 10.0f,
            0.0f,
            1.0f);
        buf_channelGains[i] = slider * cv;
    }

        // send gains
    for (int i = 0; i < numChannels; ++i) {
        const float slider = TBase::params[i + SEND0_PARAM].value;
        buf_channelSendGains[i] = slider;
    }

    // fill buf_leftPanGains and buf_rightPanGains
    for (int i = 0; i < numChannels; ++i) {
        const float balance = TBase::params[i + PAN0_PARAM].value;
        const float cv = TBase::inputs[i + PAN0_INPUT].value;
        const float panValue = std::clamp(balance + cv / 5, -1, 1);
        assert(panValue >= -1);
        assert(panValue <= 1);
        buf_leftPanGains[i] = LookupTable<float>::lookup(*panL, panValue);
        buf_rightPanGains[i] = LookupTable<float>::lookup(*panR, panValue);
    }

    bool anySolo = false;
    for (int i = 0; i < numChannels; ++i) {
        if (TBase::params[i + SOLO0_PARAM].value > .5f) {
            anySolo = true;
            break;
        }
    }

    if (anySolo) {
        for (int i = 0; i < numChannels; ++i) {
            buf_muteInputs[i] = TBase::params[i + SOLO0_PARAM].value;
        }
    } else {
        for (int i = 0; i < numChannels; ++i) {
            bool isMute = (TBase::params[i + MUTE0_PARAM].value > .5f) ||
                (TBase::inputs[i + MUTE0_INPUT].value > 2);
            buf_muteInputs[i] = isMute ? 0.f : 1.f;
        }
    }
    antiPop.step(buf_muteInputs);
}

template <class TBase>
inline void Mix4<TBase>::init()
{
    const int divRate = 4;
    divider.setup(divRate, [this, divRate] {
        this->stepn(divRate);
        });

    // 400 was smooth, 100 popped
    antiPop.setCutoff(1.0f / 100.f);
}

template <class TBase>
inline void Mix4<TBase>::step()
{
    divider.step();

    // fill buf_inputs
    for (int i = 0; i < numChannels; ++i) {
        buf_inputs[i] = TBase::inputs[i + AUDIO0_INPUT].value;
    }

    // compute buf_channelOuts
    for (int i = 0; i < numChannels; ++i) {
        const float muteValue = antiPop.get(i);
        buf_channelOuts[i] = buf_inputs[i] * buf_channelGains[i] * muteValue;
    }

    // compute and output master outputs
    float left = 0, right = 0;
    float lSend = 0, rSend = 0;
    if (expansionInputs) {
        left  = expansionInputs[0];
        right = expansionInputs[1];
        lSend = expansionInputs[2];
        rSend = expansionInputs[3];
    }

    for (int i = 0; i < numChannels; ++i) {
        left += buf_channelOuts[i] * buf_leftPanGains[i];
        lSend += buf_channelOuts[i] * buf_leftPanGains[i] * buf_channelSendGains[i];
        right += buf_channelOuts[i] * buf_rightPanGains[i];
        rSend += buf_channelOuts[i] * buf_rightPanGains[i] * buf_channelSendGains[i];
    }

    // output the masters
    if (expansionOutputs) {
        expansionOutputs[0] = left;
        expansionOutputs[1] = right;
        expansionOutputs[2] = lSend;
        expansionOutputs[3] = rSend;
    }
#if 0
    const float masterMuteValue = antiPop.get(8);
    const float masterGain = buf_masterGain * masterMuteValue;
    TBase::outputs[LEFT_OUTPUT].value = left * masterGain + TBase::inputs[LEFT_EXPAND_INPUT].value;
    TBase::outputs[RIGHT_OUTPUT].value = right * masterGain + TBase::inputs[RIGHT_EXPAND_INPUT].value;
#endif
    // output channel outputs
    for (int i = 0; i < numChannels; ++i) {
        TBase::outputs[i + CHANNEL0_OUTPUT].value = buf_channelOuts[i];
    }
}


template <class TBase>
inline void Mix4<TBase>::setExpansionInputs(const float* p)
{
    expansionInputs = p;
}

template <class TBase>
inline void Mix4<TBase>::setExpansionOutputs(float* p)
{
    expansionOutputs = p;
}

template <class TBase>
int Mix4Description<TBase>::getNumParams()
{
    return Mix4<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config Mix4Description<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {

   //     case Mix4<TBase>::MASTER_VOLUME_PARAM:
   //         ret = {0, 1, .8f, "Master Vol"};
   //         break;
    //    case Mix4<TBase>::MASTER_MUTE_PARAM:
    //        ret = {0, 1, 0, "Master Mute"};
    //        break;
        case Mix4<TBase>::GAIN0_PARAM:
            ret = {0, 1, .8f, "Level 1"};
            break;
        case Mix4<TBase>::GAIN1_PARAM:
            ret = {0, 1, .8f, "Level 2"};
            break;
        case Mix4<TBase>::GAIN2_PARAM:
            ret = {0, 1, .8f, "Level 3"};
            break;
        case Mix4<TBase>::GAIN3_PARAM:
            ret = {0, 1, .8f, "Level 4"};
            break;
      
        case Mix4<TBase>::PAN0_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 1"};
            break;
        case Mix4<TBase>::PAN1_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 2"};
            break;
        case Mix4<TBase>::PAN2_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 3"};
            break;
        case Mix4<TBase>::PAN3_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 4"};
            break;
    
        case Mix4<TBase>::MUTE0_PARAM:
            ret = {0, 1.0f, 0, "Mute  1"};
            break;
        case Mix4<TBase>::MUTE1_PARAM:
            ret = {0, 1.0f, 0, "Mute  2"};
            break;
        case Mix4<TBase>::MUTE2_PARAM:
            ret = {0, 1.0f, 0, "Mute  3"};
            break;
        case Mix4<TBase>::MUTE3_PARAM:
            ret = {0, 1.0f, 0, "Mute  4"};
            break;
   
        case Mix4<TBase>::SOLO0_PARAM:
            ret = {0, 1.0f, 0, "Solo  1"};
            break;
        case Mix4<TBase>::SOLO1_PARAM:
            ret = {0, 1.0f, 0, "Solo  2"};
            break;
        case Mix4<TBase>::SOLO2_PARAM:
            ret = {0, 1.0f, 0, "Solo  3"};
            break;
        case Mix4<TBase>::SOLO3_PARAM:
            ret = {0, 1.0f, 0, "Solo  4"};
            break;

        case Mix4<TBase>::SEND0_PARAM:
            ret = {0, 1.0f, 0, "Send 1"};
            break;
        case Mix4<TBase>::SEND1_PARAM:
            ret = {0, 1.0f, 0, "Send 2"};
            break;
        case Mix4<TBase>::SEND2_PARAM:
            ret = {0, 1.0f, 0, "Send 3"};
            break;
        case Mix4<TBase>::SEND3_PARAM:
            ret = {0, 1.0f, 0, "Send 4"};
            break;
   
        default:
            assert(false);
    }
    return ret;
}

#ifdef _USEAVX


template <class TBase>
inline void Mix4<TBase>::step()
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


