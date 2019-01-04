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
#ifdef _COLORS
    p->addModel(modelColoredNoiseModule);
 #endif
#ifdef _EV3 
    p->addModel(modelEV3Module);
    #endif
#ifdef _GROWLER
    p->addModel(modelVocalFilterModule);
#endif
#ifdef _FUN
    p->addModel(modelFunVModule);
    #endif
#ifdef _GRAY
    p->addModel(modelGrayModule);
    #endif
#ifdef _FORMANTS
    p->addModel(modelVocalModule);
    #endif
#ifdef _LFN
    p->addModel(modelLFNModule); 
<<<<<<< HEAD
    #endif
#ifdef _SHAPER
=======
#ifdef _SUPER
    p->addModel(modelSuperModule);
#endif
>>>>>>> master
    p->addModel(modelShaperModule);
    #endif
#ifdef _TBOOST
    p->addModel(modelThreadBoostModule);
#endif

    p->addModel(modelCHBgModule);

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