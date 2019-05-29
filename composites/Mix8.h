
#pragma once

#include "Divider.h"
#include "IComposite.h"
#include "MultiLag.h"
#include "ObjectCache.h"
#include "SqMath.h"

#include <assert.h>
#include <memory>

#ifdef __V1
namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = rack::engine::Module;
#else
namespace rack {
    struct Module;
};
using Module = rack::Module;
#endif


template <class TBase>
class Mix8Description : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};


/**
 * CPU usage, straight AS copy: 298
 *  with all the master and mute logic hooked up, 299
 * with pan lookup: 44
 * add /4 process for cv : 19
 * add the master mute and expand: 19.6
 * with final features: 25
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
    Mix8(Module * module) : TBase(module)
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
        MASTER_VOLUME_PARAM,
        MASTER_MUTE_PARAM,
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
        MUTE0_PARAM,
        MUTE1_PARAM,
        MUTE2_PARAM,
        MUTE3_PARAM,
        MUTE4_PARAM,
        MUTE5_PARAM,
        MUTE6_PARAM,
        MUTE7_PARAM,
        SOLO0_PARAM,
        SOLO1_PARAM,
        SOLO2_PARAM,
        SOLO3_PARAM,
        SOLO4_PARAM,
        SOLO5_PARAM,
        SOLO6_PARAM,
        SOLO7_PARAM,
        SEND0_PARAM,
        SEND1_PARAM,
        SEND2_PARAM,
        SEND3_PARAM,
        SEND4_PARAM,
        SEND5_PARAM,
        SEND6_PARAM,
        SEND7_PARAM,
        RETURN_GAIN_PARAM,
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

        MUTE0_INPUT,
        MUTE1_INPUT,
        MUTE2_INPUT,
        MUTE3_INPUT,
        MUTE4_INPUT,
        MUTE5_INPUT,
        MUTE6_INPUT,
        MUTE7_INPUT,

        RIGHT_EXPAND_INPUT,
        LEFT_EXPAND_INPUT,
        LEFT_RETURN_INPUT,
        RIGHT_RETURN_INPUT,
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
        LEFT_SEND_OUTPUT,
        RIGHT_SEND_OUTPUT,
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

    float buf_inputs[numChannels] = {0};
    float buf_channelGains[numChannels] = {0};
    float buf_channelOuts[numChannels] = {0};
    float buf_leftPanGains[numChannels] = {0};
    float buf_rightPanGains[numChannels] = {0};
    float buf_channelSendGains[numChannels] = {0};

    /** 
     * allocate extra bank for the master mute
     */
    float buf_muteInputs[numChannels + 4];
    float buf_masterGain=0;
    float buf_auxReturnGain = 0;

private:
    Divider divider;

    /**
     * 8 input channels and one master
     */
    MultiLPF<12> antiPop;
    std::shared_ptr<LookupTableParams<float>> panL = ObjectCache<float>::getMixerPanL();
    std::shared_ptr<LookupTableParams<float>> panR = ObjectCache<float>::getMixerPanR();
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

#if 0
static inline float PanL(float balance, float cv)
{ // -1...+1
    float p, inp;
    inp = balance + cv / 5;
    p = M_PI * (std::clamp(inp, -1.0f, 1.0f) + 1) / 4;
    return ::cos(p);
}

static inline float PanR(float balance, float cv)
{
    float p, inp;
    inp = balance + cv / 5;
    p = M_PI * (std::clamp(inp, -1.0f, 1.0f) + 1) / 4;
    return ::sin(p);
}
#endif

template <class TBase>
inline void Mix8<TBase>::stepn(int div)
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

    buf_masterGain = TBase::params[MASTER_VOLUME_PARAM].value;
    buf_auxReturnGain = TBase::params[RETURN_GAIN_PARAM].value;

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
        buf_leftPanGains[i] = LookupTable<float>::lookup(*panL, panValue);
        buf_rightPanGains[i] = LookupTable<float>::lookup(*panR, panValue);
    }

    buf_masterGain = TBase::params[MASTER_VOLUME_PARAM].value;

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
            const bool muteActivated = ((TBase::params[i + MUTE0_PARAM].value > .5f) ||
                (TBase::inputs[i + MUTE0_INPUT].value > 2));
            buf_muteInputs[i] = muteActivated ? 0.f : 1.f;
           // buf_muteInputs[i] = 1.0f - TBase::params[i + MUTE0_PARAM].value;       // invert mute
        }
    }
    buf_muteInputs[8] = 1.0f - TBase::params[MASTER_MUTE_PARAM].value;
    antiPop.step(buf_muteInputs);
}

template <class TBase>
inline void Mix8<TBase>::init()
{
    const int divRate = 4;
    divider.setup(divRate, [this, divRate] {
        this->stepn(divRate);
        });

    // 400 was smooth, 100 popped
    antiPop.setCutoff(1.0f / 100.f);
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
        const float muteValue = antiPop.get(i);
        buf_channelOuts[i] = buf_inputs[i] * buf_channelGains[i] * muteValue;
    }

    // compute and output master outputs
    float left = 0, right = 0;
    float lSend = 0, rSend = 0;
    for (int i = 0; i < numChannels; ++i) {
        left += buf_channelOuts[i] * buf_leftPanGains[i];
        right += buf_channelOuts[i] * buf_rightPanGains[i];

        lSend += buf_channelOuts[i] * buf_leftPanGains[i] * buf_channelSendGains[i];
        rSend += buf_channelOuts[i] * buf_rightPanGains[i] * buf_channelSendGains[i];
    }

    left += TBase::inputs[LEFT_RETURN_INPUT].value * buf_auxReturnGain;
    right += TBase::inputs[RIGHT_RETURN_INPUT].value * buf_auxReturnGain;

    // output the masters
    const float masterMuteValue = antiPop.get(8);
    const float masterGain = buf_masterGain * masterMuteValue;
    TBase::outputs[LEFT_OUTPUT].value = left * masterGain + TBase::inputs[LEFT_EXPAND_INPUT].value;
    TBase::outputs[RIGHT_OUTPUT].value = right * masterGain + TBase::inputs[RIGHT_EXPAND_INPUT].value;

    TBase::outputs[LEFT_SEND_OUTPUT].value = lSend;
    TBase::outputs[RIGHT_SEND_OUTPUT].value = rSend;

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

        case Mix8<TBase>::MASTER_VOLUME_PARAM:
            ret = {0, 1, .8f, "Master Vol"};
            break;
        case Mix8<TBase>::MASTER_MUTE_PARAM:
            ret = {0, 1, 0, "Master Mute"};
            break;
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
        case Mix8<TBase>::MUTE0_PARAM:
            ret = {0, 1.0f, 0, "Mute  1"};
            break;
        case Mix8<TBase>::MUTE1_PARAM:
            ret = {0, 1.0f, 0, "Mute  2"};
            break;
        case Mix8<TBase>::MUTE2_PARAM:
            ret = {0, 1.0f, 0, "Mute  3"};
            break;
        case Mix8<TBase>::MUTE3_PARAM:
            ret = {0, 1.0f, 0, "Mute  4"};
            break;
        case Mix8<TBase>::MUTE4_PARAM:
            ret = {0, 1.0f, 0, "Mute  5"};
            break;
        case Mix8<TBase>::MUTE5_PARAM:
            ret = {0, 1.0f, 0, "Mute  6"};
            break;
        case Mix8<TBase>::MUTE6_PARAM:
            ret = {0, 1.0f, 0, "Mute  7"};
            break;
        case Mix8<TBase>::MUTE7_PARAM:
            ret = {0, 1.0f, 0, "Mute  8"};
            break;
        case Mix8<TBase>::SOLO0_PARAM:
            ret = {0, 1.0f, 0, "Solo  1"};
            break;
        case Mix8<TBase>::SOLO1_PARAM:
            ret = {0, 1.0f, 0, "Solo  2"};
            break;
        case Mix8<TBase>::SOLO2_PARAM:
            ret = {0, 1.0f, 0, "Solo  3"};
            break;
        case Mix8<TBase>::SOLO3_PARAM:
            ret = {0, 1.0f, 0, "Solo  4"};
            break;
        case Mix8<TBase>::SOLO4_PARAM:
            ret = {0, 1.0f, 0, "Solo  5"};
            break;
        case Mix8<TBase>::SOLO5_PARAM:
            ret = {0, 1.0f, 0, "Solo  6"};
            break;
        case Mix8<TBase>::SOLO6_PARAM:
            ret = {0, 1.0f, 0, "Solo  7"};
            break;
        case Mix8<TBase>::SOLO7_PARAM:
            ret = {0, 1.0f, 0, "Solo  8"};
            break;
        case Mix8<TBase>::SEND0_PARAM:
            ret = {0, 1.0f, 0, "Send 1"};
            break;
        case Mix8<TBase>::SEND1_PARAM:
            ret = {0, 1.0f, 0, "Send 2"};
            break;
        case Mix8<TBase>::SEND2_PARAM:
            ret = {0, 1.0f, 0, "Send 3"};
            break;
        case Mix8<TBase>::SEND3_PARAM:
            ret = {0, 1.0f, 0, "Send 4"};
            break;
        case Mix8<TBase>::SEND4_PARAM:
            ret = {0, 1.0f, 0, "Send 5"};
            break;
        case Mix8<TBase>::SEND5_PARAM:
            ret = {0, 1.0f, 0, "Send 6"};
            break;
        case Mix8<TBase>::SEND6_PARAM:
            ret = {0, 1.0f, 0, "Send 7"};
            break;
        case Mix8<TBase>::SEND7_PARAM:
            ret = {0, 1.0f, 0, "Send 8"};
            break;
        case Mix8<TBase>::RETURN_GAIN_PARAM:
            ret = {0, 1.0f, 0, "Return Gain"};
            break;
        default:
            assert(false);
    }
    return ret;
}


