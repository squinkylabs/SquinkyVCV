
#pragma once

#include "CommChannels.h"
#include "Divider.h"
#include "IComposite.h"
#include "MixHelper.h"
#include "MultiLag.h"
#include "ObjectCache.h"
#include "SqMath.h"

#include <assert.h>
#include <memory>

namespace rack {
    namespace engine {
        struct Module;
    }
}

template <class TBase>
class MixStereoDescription : public IComposite
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
    13.5 with all the features

 */

template <class TBase>
class MixStereo : public TBase
{
public:

    template<typename Q>
    friend class MixHelper;

    MixStereo(rack::engine::Module * module) : TBase(module)
    {
    }
    MixStereo() : TBase()
    {
    }
    static const int numChannels = 4;
    static const int numGroups = 2;


    /**
    * Only needs to be called once.
    */
    void init();
    void setupFilters();

    enum ParamIds
    {
        GAIN0_PARAM,
        GAIN1_PARAM,
      

        PAN0_PARAM,
        PAN1_PARAM,
     
        MUTE0_PARAM,
        MUTE1_PARAM,
        SOLO0_PARAM,
        SOLO1_PARAM,
      
        ALL_CHANNELS_OFF_PARAM, // when > .05, acts as if all channels muted.

        SEND0_PARAM,
        SEND1_PARAM,
        SENDb0_PARAM,
        SENDb1_PARAM,
       
        PRE_FADERa_PARAM,       // 0 = post, 1 = pre
        PRE_FADERb_PARAM,

        MUTE0_STATE_PARAM,
        MUTE1_STATE_PARAM,
       

        CV_MUTE_TOGGLE,

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
    
        PAN0_INPUT,
        PAN1_INPUT,
       
        MUTE0_INPUT,
        MUTE1_INPUT,
       
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
        MUTE0_LIGHT,
        MUTE1_LIGHT,
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<MixStereoDescription<TBase>>();
    }

    void setExpansionInputs(const float*);
    void setExpansionOutputs(float*);

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;
    void onSampleRateChange();

    void stepn(int steps);

    // These are all calculated in stepn from the
    // contents of filteredCV

    // TODO: this should be numGroups
    float buf_channelSendGainsALeft[numChannels] = {0};
    float buf_channelSendGainsARight[numChannels] = {0};
    float buf_channelSendGainsBLeft[numChannels] = {0};
    float buf_channelSendGainsBRight[numChannels] = {0};

    // TODO: reduce this number to what we actually need

    /**
     *      0..1 for smoothed input gain * channel mute
     *      2..3 for left  pan
     *      4..5 for right pan
     *      6..7 for mute
     */

    static const int cvOffsetGain = 0;
    static const int cvOffsetPanLeft = 2;
    static const int cvOffsetPanRight = 4;
    static const int cvOffsetMute = 6;
    static const int cvFilterSize = 8;

    // This is where we build up all the inputs to the CV filter
    float unbufferedCV[cvFilterSize] = {0};


    void _disableAntiPop();

private:
    Divider divider;



   
    MultiLPF<cvFilterSize> filteredCV;

    // TODO: pan for stereo
  //  std::shared_ptr<LookupTableParams<float>> panL = ObjectCache<float>::getMixerPanL();
  //  std::shared_ptr<LookupTableParams<float>> panR = ObjectCache<float>::getMixerPanR();

    const float* expansionInputs = nullptr;
    float* expansionOutputs = nullptr;

    // TODO:cut down mix helper size.
    MixHelper<MixStereo<TBase>> helper;

#ifdef _CHAUDIOTAPER
     std::shared_ptr<LookupTableParams<float>> taperLookupParam =  ObjectCache<float>::getAudioTaper18();
#endif
};

template <class TBase>
inline void MixStereo<TBase>::stepn(int div)
{
    // TODO: is this correct for stereo?
    //float unbufferesdCV[cvOffsetMute + 4] = {0};
    const int numUb = cvFilterSize;
    for (int i = 0; i < numUb; ++i) {
        unbufferedCV[i] = 0;
    }

    const bool moduleIsMuted = TBase::params[ALL_CHANNELS_OFF_PARAM].value > .5f;
    const bool AisPreFader = TBase::params[PRE_FADERa_PARAM].value > .5;
    const bool BisPreFader = TBase::params[PRE_FADERb_PARAM].value > .5;

    helper.procMixInputs(this);

    //helper.procMasterMute(this);

    // If the is an external solo, then mute all channels
    bool anySolo = false;
    for (int i = 0; i < numGroups; ++i) {
        assert(i + SOLO0_PARAM < NUM_PARAMS);
        if (TBase::params[i + SOLO0_PARAM].value > .5f) {
            anySolo = true;
            break;
        }
    }

    for (int channel = 0; channel < numChannels; ++channel) {
        const int group = channel / 2;
        float groupGain = 0;

        // First let's round up the channel volume
        {
            assert(group + GAIN0_PARAM < NUM_PARAMS);
            const float rawSlider = TBase::params[group + GAIN0_PARAM].value;
            const float slider = LookupTable<float>::lookup(*taperLookupParam, rawSlider);

            assert(group + LEVEL0_INPUT < NUM_INPUTS);
            const float rawCV = TBase::inputs[group + LEVEL0_INPUT].isConnected() ?
                TBase::inputs[group + LEVEL0_INPUT].value : 10.f;
            const float cv = std::clamp(
                rawCV / 10.0f,
                0.0f,
                1.0f);
            groupGain = slider * cv;
        }

        // now round up the mutes
        float rawMuteValue = 0;        // assume muted
        if (moduleIsMuted) {

        } else if (anySolo) {
            // If any channels in this module are soloed, then
            // mute any channels that aren't soled
            assert(group + SOLO0_PARAM < NUM_PARAMS);
            rawMuteValue = TBase::params[group + SOLO0_PARAM].value;
        } else {
             // The pre-calculated state in :params[i + MUTE0_STATE_PARAM] will
             // be applicable if no solo
            assert(group + MUTE0_STATE_PARAM < NUM_PARAMS);
            rawMuteValue = TBase::params[group + MUTE0_STATE_PARAM].value > .5 ? 0.f : 1.f;
        }

        groupGain *= rawMuteValue;

        // now the raw channel gains are all computed
        // TODO: split this out into a separate function, only needs to be done on groups
        assert(cvOffsetGain + group < cvFilterSize);
        assert(cvOffsetMute + group < cvFilterSize);
        unbufferedCV[cvOffsetGain + group] = groupGain;
        unbufferedCV[cvOffsetMute + group] = rawMuteValue;

        // now do the pan calculation
        {
            assert(group + PAN0_PARAM < NUM_PARAMS);
            assert(group + PAN0_INPUT < NUM_INPUTS);
            const float balance = TBase::params[group + PAN0_PARAM].value;
            const float cv = TBase::inputs[group + PAN0_INPUT].value;
            const float panValue = std::clamp(balance + cv / 5, -1, 1);


            // temporary pan hack to boostrap mixer and tests
            float panGainLeft = 1;
            float panGainRight = 1;
            if (panValue < -.9) {
                panGainRight = 0;
            } else if (panValue > .9) {
                panGainLeft = 0;
            }
            assert(cvOffsetPanRight + group < cvFilterSize);
            unbufferedCV[cvOffsetPanLeft + group] = panGainLeft * groupGain;
            unbufferedCV[cvOffsetPanRight + group] = panGainRight * groupGain;

            //unbufferedCV[cvOffsetPanLeft + i] = LookupTable<float>::lookup(*panL, panValue) * channelGain;
            //unbufferedCV[cvOffsetPanRight + i] = LookupTable<float>::lookup(*panR, panValue) * channelGain;
        }


        // TODO: precalc all the send gains

        {
            assert(group + SEND0_PARAM < NUM_PARAMS);
            assert(group + SENDb0_PARAM < NUM_PARAMS);

            const float muteValue = filteredCV.get(cvOffsetMute + group);
            const float sliderA = TBase::params[group + SEND0_PARAM].value;
            const float sliderB = TBase::params[group + SENDb0_PARAM].value;


            // TODO: we can do some main volume work ahead of time, just like the sends here
            if (!AisPreFader) {
                // post faster, gain sees mutes, faders,  pan, and send level    
                buf_channelSendGainsALeft[group] = filteredCV.get(group + cvOffsetPanLeft) * sliderA;
                buf_channelSendGainsARight[group] = filteredCV.get(group + cvOffsetPanRight) * sliderA;
            } else {
                // pre-fader fader, gain sees mutes and send only
                buf_channelSendGainsALeft[group] = muteValue * sliderA * (1.f / sqrt(2.f));
                buf_channelSendGainsARight[group] = muteValue * sliderA * (1.f / sqrt(2.f));
            }

            if (!BisPreFader) {
                // post faster, gain sees mutes, faders,  pan, and send level
                buf_channelSendGainsBLeft[group] = filteredCV.get(group + cvOffsetPanLeft) * sliderB;
                buf_channelSendGainsBRight[group] = filteredCV.get(group + cvOffsetPanRight) * sliderB;
            } else {
                // pref fader, gain sees mutes and send only
                buf_channelSendGainsBLeft[group] = muteValue * sliderB * (1.f / sqrt(2.f));
                buf_channelSendGainsBRight[group] = muteValue * sliderB * (1.f / sqrt(2.f));
            }
        }

        // refresh the solo lights
        {
            assert(group + SOLO0_PARAM < NUM_PARAMS);
            assert(group + SOLO0_LIGHT < NUM_LIGHTS);
            const float soloValue = TBase::params[group + SOLO0_PARAM].value;
            TBase::lights[group + SOLO0_LIGHT].value = (soloValue > .5f) ? 10.f : 0.f;
        }

    }
    filteredCV.step(unbufferedCV);
}

template <class TBase>
inline void MixStereo<TBase>::init()
{
    const int divRate = 4;
    divider.setup(divRate, [this, divRate] {
        this->stepn(divRate);
        });
    setupFilters();
}

template <class TBase>
inline void MixStereo<TBase>::onSampleRateChange()
{
    setupFilters();
}

template <class TBase>
inline void MixStereo<TBase>::setupFilters()
{
    // 400 was smooth, 100 popped
    const float x = TBase::engineGetSampleTime() * 44100.f / 100.f;
    //printf("using %f, calc=%f\n", x, (1.0f / 100.f)); fflush(stdout);
    filteredCV.setCutoff(x);
}

template <class TBase>
inline void MixStereo<TBase>::_disableAntiPop()
{
    filteredCV.setCutoff(0.49f);     // set it super fast
}

template <class TBase>
inline void MixStereo<TBase>::step()
{
    divider.step();

    float left = 0, right = 0;              // these variables will be summed up over all channels
    float lSend = 0, rSend = 0;
    float lSendb = 0, rSendb = 0;

    if (expansionInputs) {
        left = expansionInputs[0];
        right = expansionInputs[1];
        lSend = expansionInputs[2];
        rSend = expansionInputs[3];
        lSendb = expansionInputs[4];
        rSendb = expansionInputs[5];
    }

    for (int channel = 0; channel < numChannels; ++channel) {
        const int group = channel / 2;

        assert(channel + AUDIO0_INPUT < NUM_INPUTS);
        const float channelInput = TBase::inputs[channel + AUDIO0_INPUT].value;
    #if 0
        printf("channel input(%d) = %.2f filtered cd = %.2f / %.2f\n", channel, channelInput, 
            filteredCV.get(group + cvOffsetPanLeft),
            filteredCV.get(group + cvOffsetPanRight));
    #endif

        // sum the channel output to the masters
        left += channelInput * filteredCV.get(group + cvOffsetPanLeft);
        right += channelInput * filteredCV.get(group + cvOffsetPanRight);

        // TODO: aux sends
        lSend += channelInput * buf_channelSendGainsALeft[group];
        lSendb += channelInput * buf_channelSendGainsBLeft[group];
        rSend += channelInput * buf_channelSendGainsARight[group];
        rSendb += channelInput * buf_channelSendGainsBRight[group];

        assert(channel + CHANNEL0_OUTPUT < NUM_OUTPUTS);
        TBase::outputs[channel + CHANNEL0_OUTPUT].value = channelInput * filteredCV.get(group + cvOffsetGain);
    }

    // output the buses to the expansion port
    if (expansionOutputs) {
        expansionOutputs[0] = left;
        expansionOutputs[1] = right;
        expansionOutputs[2] = lSend;
        expansionOutputs[3] = rSend;
        expansionOutputs[4] = lSendb;
        expansionOutputs[5] = rSendb;
        //printf("sending l=%.2f r = %.2f\n", left, right); fflush(stdout);
    } 

}

template <class TBase>
inline void MixStereo<TBase>::setExpansionInputs(const float* p)
{
    expansionInputs = p;
}

template <class TBase>
inline void MixStereo<TBase>::setExpansionOutputs(float* p)
{
    expansionOutputs = p;
}

template <class TBase>
int MixStereoDescription<TBase>::getNumParams()
{
    return MixStereo<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config MixStereoDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case MixStereo<TBase>::GAIN0_PARAM:
            ret = {0, 1, 0, "Level 1"};
            break;
        case MixStereo<TBase>::GAIN1_PARAM:
            ret = {0, 1, 0, "Level 2"};
            break;

        case MixStereo<TBase>::PAN0_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 1"};
            break;
        case MixStereo<TBase>::PAN1_PARAM:
            ret = {-1.0f, 1.0f, 0.0f, "Pan 2"};
            break;

        case MixStereo<TBase>::MUTE0_PARAM:
            ret = {0, 1.0f, 0, "Mute  1"};
            break;
        case MixStereo<TBase>::MUTE1_PARAM:
            ret = {0, 1.0f, 0, "Mute  2"};
            break;

        case MixStereo<TBase>::SOLO0_PARAM:
            ret = {0, 1.0f, 0, "Solo  1"};
            break;
        case MixStereo<TBase>::SOLO1_PARAM:
            ret = {0, 1.0f, 0, "Solo  2"};
            break;
        case MixStereo<TBase>::SEND0_PARAM:
            ret = {0, 1.0f, 0, "Send 1"};
            break;
        case MixStereo<TBase>::SEND1_PARAM:
            ret = {0, 1.0f, 0, "Send 2"};
            break;
        case MixStereo<TBase>::SENDb0_PARAM:
            ret = {0, 1.0f, 0, "Send 1b"};
            break;
        case MixStereo<TBase>::SENDb1_PARAM:
            ret = {0, 1.0f, 0, "Send 2b"};
            break;
        case  MixStereo<TBase>::ALL_CHANNELS_OFF_PARAM:
            ret = {0, 1.0f, 0, "All Off"};
            break;
        case  MixStereo<TBase>::PRE_FADERa_PARAM:      // 0 = post, 1 = pre
            ret = {0, 1.0f, 0, "Pre Fader A"};
            break;
        case  MixStereo<TBase>::PRE_FADERb_PARAM:
            ret = {0, 1.0f, 0, "Pre Fader B"};
            break;
        case MixStereo<TBase>::MUTE0_STATE_PARAM:
            ret = {0, 1, 0, "MSX0"};            // not user visible
            break;
        case MixStereo<TBase>::MUTE1_STATE_PARAM:
            ret = {0, 1, 0, "MSX1"};
            break;
        case MixStereo<TBase>::CV_MUTE_TOGGLE:
            ret = {0, 1, 0, "VCTM"};
            break;
        default:
            assert(false);
    }
    return ret;
}



