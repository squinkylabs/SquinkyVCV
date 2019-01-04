#pragma once

#include "IComposite.h"
/** Wrap up all the .6/1.0 dependencies here
 */
#ifdef _V1
class SQHelper
{
public:
    static std::string assetPlugin(Plugin *plugin, const std::string& filename)
    {
        return asset::plugin(plugin, filename);
    } 
    static float engineGetSampleRate()
    {
        return context()->engine->getSampleRate();
    }
    template <typename T>

    static T* createParam(IComposite& dummy, const Vec& pos, Module* module, int paramId )
    {
        return rack::createParam<T>(pos, module, paramId);
    }

    static const NVGcolor COLOR_WHITE;
    static const NVGcolor COLOR_BLACK;

    // TODO: finish this (support other params);
    static void setupParams(IComposite& comp, Module* module)
    {
        const int n = comp.getNumParams();
        for (int i=0; i<n; ++i) {
            auto param = comp.getParam(i);
            module->params[i].setup(param.min, param.max, param.def, "fakename", "fake unit");
            //params[PITCH_PARAM].setup(-3.f, 3.f, 0.f, "Pitch", " semi", 0.f, 12.f);
        }
    }
};
#else
class SQHelper
{
public:
    static std::string assetPlugin(Plugin *plugin, const std::string& filename)
    {
        printf("calling assetPlugin with %p, %s\n",
            plugin, filename.c_str());
        fflush(stdout);
        return rack::assetPlugin(plugin, filename);
    } 
    static float engineGetSampleRate()
    {
        return rack::engineGetSampleRate();
    }

   static const NVGcolor COLOR_WHITE;
   static const NVGcolor COLOR_BLACK;

   template <typename T>
   static T* createParam(IComposite& composite, const Vec& pos, Module* module, int paramId )
   {
       const auto data = composite.getParam(paramId);
       printf("helper got param\n"); fflush(stdout);
       assert(data.min < data.max);
       assert(data.def >= data.min);
       assert(data.def <= data.max);
       return rack::createParam<T>(
           pos,
           module, 
           paramId,
           data.min, data.max, data.def
       );
   }
};
#endif
