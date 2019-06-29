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
};


template <class TMixComposite>
inline void MixHelper<TMixComposite>::procMixInputs(TMixComposite* mixer)
{
    for (int i = 0; i < TMixComposite::numChannels; ++i) {
        // pump the CV though the schmidts
      //  const bool muteCV = triggers[i].go(mixer->inputs[TMixComposite::MUTE0_INPUT + i].value);

        // combine schmidt and params into bool muted
       // const bool muted = muteCV || (mixer->params[TMixComposite::MUTE0_PARAM + i].value > .5);
     //  const bool muted =  


        // temp, just to toggle on param
        paramTriggers[i].go(10 * mixer->params[TMixComposite::MUTE0_PARAM + i].value);
        const bool paramTriggered = paramTriggers[i].trigger();
        if (paramTriggered) {
            bool muted = mixer->params[TMixComposite::MUTE0_STATE_PARAM + i].value > .5;
            muted = !muted;
            mixer->params[TMixComposite::MUTE0_STATE_PARAM + i].value = muted ? 1.f : 0.f;
        }
       // bool paramTriggered = paramTriggers[i].
      //  bool muted = false;
       // write that out to param

       // mixer->params[TMixComposite::MUTE0_STATE_PARAM + i].value = muted ? 1.f : 0.f;
    }
}