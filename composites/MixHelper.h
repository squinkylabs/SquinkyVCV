#pragma once

#include "GateTrigger.h"

#define _CHAUDIOTAPER

/**
 * template class must provide:
        MUTE0_PARAM..3
        MUTE0_STATE_PARAM
        params

        numChannels
 */
template <class TMixComposite>
class MixHelper
{
public:
    MixHelper()
    {

    }
    /**
     * input: params[MUTE_PARAM];
                inputs[MUTE0_INPUT]
     * output params[MUTE0_STATE_PARAM]
     */
    void procMixInputs(TMixComposite*);

    void procMasterMute(TMixComposite*);
private:
    /**
     * local state tracking stuff. index is usually channel nubmer,
     * but for master it's 4.
     */
    GateTrigger inputTriggers[TMixComposite::numChannels + 1] = {};
    GateTrigger paramTriggers[TMixComposite::numChannels + 1] = {};
    bool cvWasHigh[TMixComposite::numChannels + 1] = {false};

    void procOneMute(
        int index,              // index into my input triggers, etc..
        TMixComposite*, 
        int muteParam, 
        int muteStateParm, 
        int light, 
        bool cvMuteToggle,
        int cvInput);
};


template <class TMixComposite>
inline void MixHelper<TMixComposite>::procMixInputs(TMixComposite* mixer)
{
    const bool cvToggleMode = mixer->params[TMixComposite::CV_MUTE_TOGGLE].value > .5;
    for (int i = 0; i < TMixComposite::numChannels; ++i) {
        procOneMute(
            i,
            mixer,
            TMixComposite::MUTE0_PARAM + i,
            TMixComposite::MUTE0_STATE_PARAM + i,
            TMixComposite::MUTE0_LIGHT + i,
            cvToggleMode,
            TMixComposite::MUTE0_INPUT + i
            );
    }
}

template <class TMixComposite>
inline void MixHelper<TMixComposite>::procMasterMute(TMixComposite* mixer)
{
    const bool cvToggleMode = mixer->params[TMixComposite::CV_MUTE_TOGGLE].value > .5;
    procOneMute(
        4,
        mixer,
        TMixComposite::MASTER_MUTE_PARAM,
        TMixComposite::MASTER_MUTE_STATE_PARAM,
        TMixComposite::MUTE_MASTER_LIGHT,
        cvToggleMode,
        -1);
}

template <class TMixComposite>
inline void MixHelper<TMixComposite>::procOneMute(
    int index,
    TMixComposite* mixer,
    int muteParam,
    int muteStateParm,
    int light,
    bool cvMuteToggle,
    int cvInput)
{
    bool muted = mixer->params[muteStateParm].value > .5;

    // run the mute param though a gate trigger. Don't need schmidt, but the edge
    // detector is useful here.
    paramTriggers[index].go(10 * mixer->params[muteParam].value);

    const bool paramTriggered = paramTriggers[index].trigger();
    if (paramTriggered) {
        muted = !muted;
    }

    // look for change in mute CV. This is to keep the params
    // from fighting the CV. If CV doesn't change, params can win.
    // master has no CV, so will be -1
    if (cvInput >= 0) {
        inputTriggers[index].go(mixer->inputs[cvInput].value);
    }
    const bool inputCVActive = inputTriggers[index].gate();
   // const bool debug
    if (inputCVActive != cvWasHigh[index]) {
        if (cvMuteToggle) {
            if (inputCVActive) {
                muted = !muted;
            }
        } else {
            muted = inputCVActive;
        }
        cvWasHigh[index] = inputCVActive;
    }

    // set the final mute state
    mixer->params[muteStateParm].value = muted ? 1.f : 0.f;
    mixer->lights[light].value = muted ? 10.f : 0.f;
}
//proc1(mixer, muteparam, mutesstatusparam, light)


