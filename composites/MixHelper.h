#pragma once

#include "GateTrigger.h"

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
private:
    GateTrigger inputTriggers[TMixComposite::numChannels] = {};
    GateTrigger paramTriggers[TMixComposite::numChannels] = {};
    bool cvWasHigh[TMixComposite::numChannels] = {false};
};


template <class TMixComposite>
inline void MixHelper<TMixComposite>::procMixInputs(TMixComposite* mixer)
{
    const bool cvToggleMode = mixer->params[TMixComposite::CV_MUTE_TOGGLE].value > .5;
    for (int i = 0; i < TMixComposite::numChannels; ++i) {

        bool muted = mixer->params[TMixComposite::MUTE0_STATE_PARAM + i].value > .5;

        // run the mute param though a gate trigger. Don't need schmidt, but the edge
        // detector is useful here.
        paramTriggers[i].go(10 * mixer->params[TMixComposite::MUTE0_PARAM + i].value);
        const bool paramTriggered = paramTriggers[i].trigger();
        if (paramTriggered) {
            muted = !muted;
        }

        // look for change in mute CV. This is to keep the params
        // from fighting the CV. If CV doesn't change, params can win.
        inputTriggers[i].go(mixer->inputs[TMixComposite::MUTE0_INPUT + i].value);
        const bool inputCVActive = inputTriggers[i].gate();
       // const bool debug
        if (inputCVActive != cvWasHigh[i]) {
            if (cvToggleMode) {
                if (inputCVActive) {
                    muted = !muted;
                }
            } else {
                muted = inputCVActive;
            }
            cvWasHigh[i] = inputCVActive;
        }

        // set the final mute state
        mixer->params[TMixComposite::MUTE0_STATE_PARAM + i].value = muted ? 1.f : 0.f;


       // bool paramTriggered = paramTriggers[i].
      //  bool muted = false;
       // write that out to param

       // mixer->params[TMixComposite::MUTE0_STATE_PARAM + i].value = muted ? 1.f : 0.f;
    }
}