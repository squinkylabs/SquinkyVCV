#include "rack.hpp"

//#define _GMR
#define _CHB
#define _EV3
//#define _SUPER
//#define _SEQ

using namespace rack;
extern Plugin *plugin;
extern Model *modelBootyModule;
extern Model *modelVocalModule;
extern Model *modelVocalFilterModule;
extern Model *modelColoredNoiseModule;
extern Model *modelTremoloModule;
extern Model *modelThreadBoostModule;
extern Model *modelLFNModule;
#ifdef _CHB
extern Model *modelCHBModule;
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
extern Model *modelFunVModule;
#ifdef _EV3
extern Model *modelEV3Module;
#endif
extern Model *modelGrayModule;
extern Model *modelShaperModule;
#ifdef _DG
extern Model *modelDGModule;
#endif
extern Model *modelBlankModule;
#ifdef _SUPER
extern Model *modelSuperModule;
extern Model *modelKSModule;
#endif
#ifdef _SEQ
extern Model *modelSequencerModule;
#endif



