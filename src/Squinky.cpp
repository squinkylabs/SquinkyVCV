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

    assert(modelBootyModule);
    p->addModel(modelBootyModule);

#ifdef _EXP
    assert(modelVocalModule);
    p->addModel(modelVocalModule);
    assert(modelVocalFilterModule);
    p->addModel(modelVocalFilterModule);
#endif
}