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

    p->addModel(modelBootyModule);
    p->addModel(modelCHBModule);
    p->addModel(modelTremoloModule);
    p->addModel(modelColoredNoiseModule);
    p->addModel(modelEV3Module);
    p->addModel(modelVocalFilterModule);
    p->addModel(modelFunVModule);
    p->addModel(modelGrayModule);
    p->addModel(modelVocalModule);
    p->addModel(modelLFNModule); 
#ifdef _SUPER
    p->addModel(modelSuperModule);
#endif
    p->addModel(modelShaperModule);
    p->addModel(modelThreadBoostModule);

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


#ifdef _SINK
    p->addModel(modelKSModule);
#endif
#ifdef _CH10
    p->addModel(modelCH10Module);
#endif
}