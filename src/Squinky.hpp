#include "rack.hpp"

using namespace rack;
extern Plugin *plugin;
extern Model *modelBootyModule;
extern Model *modelVocalModule;
extern Model *modelVocalFilterModule;
extern Model *modelColoredNoiseModule;
#ifdef _CPU_HOG
extern Model *modelCPU_HogModule;
#endif
#ifdef _EXP
extern Model *modelThreadBoostModule;
#endif

