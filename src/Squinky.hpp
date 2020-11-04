#pragma once
#include "rack.hpp"
// #define _TIME_DRAWING

#ifdef _SEQQ
dd 
#endif

 #define _XFORM

#define _FUN        // works with 1.0
#define _LFN
#define _FORMANTS
//#define _SHAPER
#define _CHB
#define _GRAY
#define _TREM
#define _COLORS
#define _EV3
//#define _SUPER
#define _BOOTY
#define _GROWLER
//#define _SLEW
#define _MIX8
//#define _FILT
//#define _CH10
//#define _LFNB
#define _MIX_STEREO
#define _USERKB
//#define _CHAOS
//#define _DG
//#define _F2
#define _F4

//#define _BLANKMODULE

#define _MIX4
#define _MIXM
#define _DTMODULE

// #define _SEQ4
#define _WVCO
#define _SUB
#define _SINES
// #define _F4

using namespace ::rack;

extern plugin::Plugin *pluginInstance;
extern Model *modelBootyModule;
extern Model *modelColoredNoiseModule;
extern Model *modelTremoloModule;
extern Model *modelThreadBoostModule;
extern Model *modelLFNModule;
extern Model *modelLFNBModule;
extern Model *modelCHBModule;
extern Model *modelCHBgModule;

#ifdef _F2
extern Model *modelF2Module;
#endif

extern Model *modelCompressorModule;

#ifdef _F4
    extern Model *modelF4Module;
#endif
#ifdef _FORMANTS
    extern Model *modelVocalFilterModule;
#endif
#ifdef _GROWLER
    extern Model *modelVocalModule;
#endif
#ifdef _GMR
extern Model *modelGMRModule;
#endif
#ifdef _CPU_HOG
extern Model *modelCPU_HogModule;
#endif
#ifdef _EV
extern Model *modelEVModule;
#endif
#ifdef _FUN
extern Model *modelFunVModule;
#endif
#ifdef _EV3
extern Model *modelEV3Module;
#endif
#ifdef _GRAY
extern Model *modelGrayModule;
#endif
#ifdef _SHAPER
extern Model *modelShaperModule;
#endif
#ifdef _DG
extern Model *modelDGModule;
#endif
#ifdef _BLANKMODULE
extern Model *modelBlankModule;
#endif
#ifdef _SUPER
extern Model *modelSuperModule;
#endif
#ifdef _CH10
extern Model *modelCH10Module;
#endif

#ifdef _SINK
extern Model *modelKSModule;
#endif
#ifdef _SEQQ
extern Model *modelSequencerModule;
#endif
#ifdef _SEQ4
extern Model *modelSequencer4Module;
#endif
#ifdef _SLEW
extern Model *modelSlew4Module;
#endif
#ifdef _MIX8
extern Model *modelMix8Module;
#endif
#ifdef _MIX4
extern Model *modelMix4Module;
#endif
#ifdef _MIXM
extern Model *modelMixMModule;
#endif
#ifdef _MIX_STEREO
extern Model* modelMixStereoModule;
#endif
#ifdef _FILT
extern Model *modelFiltModule;
#endif
#ifdef _DTMODULE
extern Model* modelDrumTriggerModule;
#endif
#ifdef _CHAOS
extern Model* modelChaosKittyModule;
#endif
#ifdef _WVCO
extern Model *modelWVCOModule;
#endif
#ifdef _SUB
extern Model *modelSubModule;
#endif
#ifdef _SINES
extern Model *modelSinesModule;
#endif
extern Model *modelBasicModule;
extern Model *modelDividerXModule;


