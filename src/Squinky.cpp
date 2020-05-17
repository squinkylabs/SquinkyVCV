// plugin main
#include "Squinky.hpp"
//#include "SqTime.h"
#include "ctrl/SqHelper.h"


// The plugin-wide instance of the Plugin class
Plugin *pluginInstance = nullptr;

/**
 * Here we register the whole plugin, which may have more than one module in it.
 */
void init (::rack::Plugin *p)
{
    pluginInstance = p;
    p->slug = "squinkylabs-plug1";
    p->version = TOSTRING(VERSION);

#ifdef _BOOTY
    p->addModel(modelBootyModule);
#endif
#ifdef _CHB
    p->addModel(modelCHBModule);
#endif
#ifdef _TREM
    p->addModel(modelTremoloModule);
#endif
#ifdef _COLORS
    p->addModel(modelColoredNoiseModule);
 #endif
#ifdef _EV3 
    p->addModel(modelEV3Module);
    #endif
#ifdef _FORMANTS
    p->addModel(modelVocalFilterModule);
#endif
#ifdef _FUN
    p->addModel(modelFunVModule);
    #endif
#ifdef _GRAY
    p->addModel(modelGrayModule);
    #endif
#ifdef _GROWLER
    p->addModel(modelVocalModule);
    #endif
#ifdef _LFN
    p->addModel(modelLFNModule); 
#endif
#ifdef _LFNB
    p->addModel(modelLFNBModule); 
#endif
#ifdef _MIX8
    p->addModel(modelMix8Module);
#endif
#ifdef _SUPER
    p->addModel(modelSuperModule);
#endif
#ifdef _SHAPER
    p->addModel(modelShaperModule);
#endif
#ifdef _SLEW
    // Slade
    p->addModel(modelSlew4Module);
#endif
#ifdef _FILT
    // Stairway
    p->addModel(modelFiltModule);
#endif
#ifdef _TBOOST
    p->addModel(modelThreadBoostModule);
#endif
#ifdef _CHBG
    p->addModel(modelCHBgModule);
#endif

#ifdef _SEQ
    assert(modelSequencerModule);
    p->addModel(modelSequencerModule);
#endif
#ifdef _GMR
    p->addModel(modelGMRModule);
#endif
#ifdef _EV
    p->addModel(modelEVModule);
#endif
#ifdef _DG
    p->addModel(modelDGModule);
#endif
#ifdef _BLANKMODULE
    p->addModel(modelBlankModule);
#endif
#ifdef _SINK
    p->addModel(modelKSModule);
#endif
#ifdef _CH10
    p->addModel(modelCH10Module);
#endif
#ifdef _MIX4
    p->addModel(modelMix4Module);
#endif
#ifdef _MIXM
    p->addModel(modelMixMModule);
#endif
#ifdef _MIX_STEREO
    p->addModel(modelMixStereoModule);
#endif
#ifdef _DTMODULE
    p->addModel(modelDrumTriggerModule);
#endif
#ifdef _SEQ4
p->addModel(modelSequencer4Module);
#endif
#ifdef _CHAOS
p->addModel(modelChaosKittyModule);
#endif
#ifdef _WVCO
p->addModel(modelWVCOModule);
#endif
#ifdef _SUB
p->addModel(modelSubModule);
#endif


}

const NVGcolor SqHelper::COLOR_GREY = nvgRGB(0x80, 0x80, 0x80);
const NVGcolor SqHelper::COLOR_WHITE = nvgRGB(0xff, 0xff, 0xff);
const NVGcolor SqHelper::COLOR_BLACK = nvgRGB(0,0,0);
const NVGcolor SqHelper::COLOR_SQUINKY = nvgRGB(0x30, 0x7d, 0xee);

int soloStateCount = 0;

#ifdef _TIME_DRAWING
#include "SqTime.h"
#ifdef _USE_WINDOWS_PERFTIME
    double SqTime::frequency = 0;
#endif
#endif