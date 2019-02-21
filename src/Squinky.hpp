#include "rack.hpp"

#include "componentlibrary.hpp"

#ifndef __V1
#define _SEQ        // just for test
#endif


#define _FUN        // works with 1.0
#define _LFN
#define _FORMANTS
#define _SHAPER
#define _CHB
#define _GRAY
#define _TREM
#define _COLORS
#define _EV3
#define _SUPER
//#define _CH10

#ifndef __V1
    #define _EV3
    #define _CHB
    #define _CHBG
    #define _LFN
    #define _COLORS
    #define _GRAY
    #define _SUPER
    #define _GROWLER
    #define _FORMANTS
    #define _TBOOST
    #define _BOOTY
    #define _TREM
#endif


using namespace rack;
extern Plugin *pluginInstance;
extern Model *modelBootyModule;
extern Model *modelColoredNoiseModule;
extern Model *modelTremoloModule;
extern Model *modelThreadBoostModule;
extern Model *modelLFNModule;
extern Model *modelCHBModule;
extern Model *modelCHBgModule;

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
#ifdef _BLANK
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
#ifdef _SEQ
extern Model *modelSequencerModule;
#endif



