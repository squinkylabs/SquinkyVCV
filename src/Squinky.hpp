#include "rack.hpp"

//#define _GMR
#define _SUPER
//#define _SEQ
#define _CH10

using namespace rack;
extern Plugin *plugin;
extern Model *modelBootyModule;
extern Model *modelVocalModule;
extern Model *modelVocalFilterModule;
extern Model *modelColoredNoiseModule;
extern Model *modelTremoloModule;
extern Model *modelThreadBoostModule;
extern Model *modelLFNModule;
extern Model *modelCHBModule;

#ifdef _GMR
extern Model *modelGMRModule;
#endif
#ifdef _CPU_HOG
extern Model *modelCPU_HogModule;
#endif
#ifdef _EV
extern Model *modelEVModule;
#endif
extern Model *modelFunVModule;
extern Model *modelEV3Module;
extern Model *modelGrayModule;
extern Model *modelShaperModule;
#ifdef _DG
extern Model *modelDGModule;
#endif
extern Model *modelBlankModule;
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



