#include "rack.hpp"

//#define _GMR
#define _CHB

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



