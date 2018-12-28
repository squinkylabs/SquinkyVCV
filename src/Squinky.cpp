// plugin main
#include "Squinky.hpp"


// The plugin-wide instance of the Plugin class
Plugin *plugin;

/**
 * Here we register the whole plugin, which may have more than one module in it.
 */
void init(rack::Plugin *p)
{
    plugin = p;
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
#ifdef _FUN
    p->addModel(modelColoredNoiseModule);
 #endif
#ifdef _FUN   
    p->addModel(modelEV3Module);
    #endif
#ifdef _FUN
    p->addModel(modelVocalFilterModule);
    #endif
#ifdef _FUN
    p->addModel(modelFunVModule);
    #endif
#ifdef _FUN
    p->addModel(modelGrayModule);
    #endif
#ifdef _FUN
    p->addModel(modelVocalModule);
    #endif
#ifdef _FUN
    p->addModel(modelLFNModule); 
    #endif
#ifdef _FUN
    p->addModel(modelShaperModule);
    #endif
#ifdef _FUN
    p->addModel(modelThreadBoostModule);
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
   // p->addModel(modelBlankModule);
#ifdef _SUPER
    p->addModel(modelSuperModule);
    p->addModel(modelKSModule);
#endif

}