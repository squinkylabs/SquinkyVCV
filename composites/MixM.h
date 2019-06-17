
#pragma once

#include "Divider.h"
#include "IComposite.h"
#include "MultiLag.h"
#include "ObjectCache.h"
#include "SqMath.h"


#include <assert.h>
#include <immintrin.h>
#include <memory>

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

template <class TBase>
class MixMDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};


/**
    Perf: 12.4 before new stuff (mix8 was 20)
    16.2 with all the features

 */

template <class TBase>
class MixM : public TBase
{
public:
    MixM(Module * module) : TBase(module)
    {
    }
    MixM() : TBase()
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
        MASTER_VOLUME_PARAM,
        MASTER_MUTE_PARAM,
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
        ALL_CHANNELS_OFF_PARAM, // when > .05, acts as if all channels muted. 

        SEND0_PARAM,
        SEND1_PARAM,
        SEND2_PARAM,
        SEND3_PARAM,

        SENDb0_PARAM,
        SENDb1_PARAM,
        SENDb2_PARAM,
        SENDb3_PARAM,

        RETURN_GAIN_PARAM,
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
        LEFT_RETURN_INPUT,
        RIGHT_RETURN_INPUT,
        LEFT_RETURNb_INPUT,
        RIGHT_RETURNb_INPUT,
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
        LEFT_SEND_OUTPUT,
        RIGHT_SEND_OUTPUT,
        LEFT_SENDb_OUTPUT,
        RIGHT_SENDb_OUTPUT,
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
        return std::make_shared<MixMDescription<TBase>>();
    }

    void setExpansionInputs(const float*);
  //  void requestModuleSolo( SoloCommands);

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    void stepn(int steps);

    float buf_inputs[numChannels] = {0};
    float buf_channelGains[numChannels] = {0};
    float buf_channelSendGains[numChannels] = {0};
    float buf_channelSendbGains[numChannels] = {0};     // second set of sends
    float buf_channelOuts[numChannels] = {0};
    float buf_leftPanGains[numChannels] = {0};
    float buf_rightPanGains[numChannels] = {0};

    /** 
     * allocate extra bank for the master mute
     */
    float buf_muteInputs[numChannels + 4] = {0};
    float buf_masterGain = 0;
    float buf_auxReturnGain = 0;

private:
    Divider divider;

    /**
     * 4 input channels and one master
     */
    MultiLPF<8> antiPop;
    std::shared_ptr<LookupTableParams<float>> panL = ObjectCache<float>::getMixerPanL();
    std::shared_ptr<LookupTableParams<float>> panR = ObjectCache<float>::getMixerPanR();

    const float* expansionInputs = nullptr;
};

#if 0
template <class TBase>
inline  void MixM<TBase>::requestModuleSolo(SoloCommands command)
{
    soloState = command;

    for (int i=0; i<4; ++i) {
        TBase::lights[i + SOLO0_LIGHT].value = (int(soloState) == i) ? 10.f : 0.f;
    }
}
#endif

template <class TBase>
inline void MixM<TBase>::stepn(int div)
{
    // fill buf_channelGains
    for (int i = 0; i < numChannels; ++i) {
        const float slider = TBase::params[i + GAIN0_PARAM].value;

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

        const float sliderb = TBase::params[i + SENDb0_PARAM].value;
        buf_channelSendbGains[i] = sliderb;

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
    buf_auxReturnGain = TBase::params[RETURN_GAIN_PARAM].value;

    // If the is an external solo, then mute all channels
#if 0
    const bool allMutedDueToSolo = (soloState == SoloCommands::SOLO_ALL);
    for (int i = 0; i < numChannels; ++i) {
        const bool muteActivated = ((TBase::params[i + MUTE0_PARAM].value > .5f) ||
            (TBase::inputs[i + MUTE0_INPUT].value > 2));
        const bool mute = allMutedDueToSolo ||
            ((i != int(soloState)) && (soloState <= SoloCommands::SOLO_3)) ||
            muteActivated; 
        
        buf_muteInputs[i] = mute ? 0.f : 1.f;
    }
#else
    bool anySolo = false;
    for (int i = 0; i < numChannels; ++i) {
        if (TBase::params[i + SOLO0_PARAM].value > .5f) {
            anySolo = true;
            break;
        }
    }
    const bool moduleIsMuted = TBase::params[ALL_CHANNELS_OFF_PARAM].value > .5f;
    if (moduleIsMuted) {
        // printf("whole module muted\n"); fflush(stdout);
        for (int i = 0; i < numChannels; ++i) {
            buf_muteInputs[i] = 0;
        } 
    } else if (anySolo) {
        // If any channels in this module are soloed, then
        // mute any channels that aren't soled
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

    //printf("buf_muteInputs = %.2f %.2f %.2f %.2f \n",buf_muteInputs[0],buf_muteInputs[1],buf_muteInputs[2],buf_muteInputs[3]);
#endif

    buf_muteInputs[4] = 1.0f - TBase::params[MASTER_MUTE_PARAM].value;
    antiPop.step(buf_muteInputs);

    for (int i=0; i<4; ++i) {
        const float soloValue = TBase::params[i + SOLO0_PARAM].value;
        TBase::lights[i + SOLO0_LIGHT].value = (soloValue > .5f) ? 10.f : 0.f;
    }
}

template <class TBase>
inline void MixM<TBase>::init()
{
    const int divRate = 4;
    divider.setup(divRate, [this, divRate] {
        this->stepn(divRate);
        });

    // 400 was smooth, 100 popped
    antiPop.setCutoff(1.0f / 100.f);
}

template <class TBase>
inline void MixM<TBase>::step()
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

    // compute and output master outputs, and send outputs
    float left = 0, right = 0;
    float lSend = 0, rSend = 0;
    float lSendb = 0, rSendb = 0;
    if (expansionInputs) {
        left = expansionInputs[0];
        right = expansionInputs[1];
        lSend = expansionInputs[2];
        rSend = expansionInputs[3];
    }
    for (int i = 0; i < numChannels; ++i) {
        left += buf_channelOuts[i] * buf_leftPanGains[i];
        lSend += buf_channelOuts[i] * buf_leftPanGains[i] * buf_channelSendGains[i];
        lSendb += buf_channelOuts[i] * buf_leftPanGains[i] * buf_channelSendbGains[i];
        right += buf_channelOuts[i] * buf_rightPanGains[i];
        rSend += buf_channelOuts[i] * buf_rightPanGains[i] * buf_channelSendGains[i];
        rSendb += buf_channelOuts[i] * buf_rightPanGains[i] * buf_channelSendbGains[i];
    }

    left += TBase::inputs[LEFT_RETURN_INPUT].value * buf_auxReturnGain;
    right += TBase::inputs[RIGHT_RETURN_INPUT].value * buf_auxReturnGain;

    // output the masters
    const float masterMuteValue = antiPop.get(numChannels);     // master is the one after
    const float masterGain = buf_masterGain * masterMuteValue;
    TBase::outputs[LEFT_OUTPUT].value = left * masterGain;
    TBase::outputs[RIGHT_OUTPUT].value = right * masterGain;


    TBase::outputs[LEFT_SEND_OUTPUT].value = lSend;
    TBase::outputs[RIGHT_SEND_OUTPUT].value = rSend;

    TBase::outputs[LEFT_SENDb_OUTPUT].value = lSendb;
    TBase::outputs[RIGHT_SENDb_OUTPUT].value = rSendb;

    // output channel outputs
    for (int i = 0; i < numChannels; ++i) {
        TBase::outputs[i + CHANNEL0_OUTPUT].value = buf_channelOuts[i];
    }
}

template <class TBase>
inline void MixM<TBase>::setExpansionInputs(const float* p)
{
    expansionInputs = p;
}

template <class TBase>
int MixMDescription<TBase>::getNumParams()
{
    return MixM<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config MixMDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {

        case MixM<TBase>::MASTER_VOLUME_PARAM:
            ret = {0, 1, .8f, "Master Vol"};
            break;
        case MixM<TBase>::MASTER_MUTE_PARAM:
            ret = {0, 1, 0, "Master Mute"};
            break;
        case MixM<TBase>::GAIN0_PARAM:
            ret = {0, 1, .8f, "Level 1"};
            break;
        case MixM<TBase>::GAIN1_PARAM:
            ret = {0, 1, .8f, "Level 2"};
            break;
        case MixM<TBase>::GAIN2_PARAM:
            ret = {0, 1, .8f, "Level 3"};
            break;
        case MixM<TBase>::GAIN3_PARAM:
            ret = {0, 1, .8f, "Level 4"};
            break;
        case MixM<TBase>::PAN0_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 1"};
            break;
        case MixM<TBase>::PAN1_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 2"};
            break;
        case MixM<TBase>::PAN2_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 3"};
            break;
        case MixM<TBase>::PAN3_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 4"};
            break;
        case MixM<TBase>::MUTE0_PARAM:
            ret = {0, 1.0f, 0, "Mute  1"};
            break;
        case MixM<TBase>::MUTE1_PARAM:
            ret = {0, 1.0f, 0, "Mute  2"};
            break;
        case MixM<TBase>::MUTE2_PARAM:
            ret = {0, 1.0f, 0, "Mute  3"};
            break;
        case MixM<TBase>::MUTE3_PARAM:
            ret = {0, 1.0f, 0, "Mute  4"};
            break;
        case MixM<TBase>::SOLO0_PARAM:
            ret = {0, 1.0f, 0, "Solo  1"};
            break;
        case MixM<TBase>::SOLO1_PARAM:
            ret = {0, 1.0f, 0, "Solo  2"};
            break;
        case MixM<TBase>::SOLO2_PARAM:
            ret = {0, 1.0f, 0, "Solo  3"};
            break;
        case MixM<TBase>::SOLO3_PARAM:
            ret = {0, 1.0f, 0, "Solo  4"};
            break;
        case MixM<TBase>::SEND0_PARAM:
            ret = {0, 1.0f, 0, "Send 1"};
            break;
        case MixM<TBase>::SEND1_PARAM:
            ret = {0, 1.0f, 0, "Send 2"};
            break;
        case MixM<TBase>::SEND2_PARAM:
            ret = {0, 1.0f, 0, "Send 3"};
            break;
        case MixM<TBase>::SEND3_PARAM:
            ret = {0, 1.0f, 0, "Send 4"};
            break;
        case MixM<TBase>::SENDb0_PARAM:
            ret = {0, 1.0f, 0, "Send 1b"};
            break;
        case MixM<TBase>::SENDb1_PARAM:
            ret = {0, 1.0f, 0, "Send 2b"};
            break;
        case MixM<TBase>::SENDb2_PARAM:
            ret = {0, 1.0f, 0, "Send 3b"};
            break;
        case MixM<TBase>::SENDb3_PARAM:
            ret = {0, 1.0f, 0, "Send 4b"};
            break;
        case MixM<TBase>::RETURN_GAIN_PARAM:
            ret = {0, 1.0f, 0, "Return Gain"};
            break;
        case MixM<TBase>::ALL_CHANNELS_OFF_PARAM:
            ret = {0, 1.0f, 0, "(All Off)"};
            break;
        default:
            assert(false);
    }
    return ret;
}

