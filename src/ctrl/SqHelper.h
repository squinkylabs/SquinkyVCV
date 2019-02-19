#pragma once

#include "app.hpp"
#include "IComposite.h"
/** Wrap up all the .6/1.0 dependencies here
 */
#ifdef __V1
class SqHelper
{
public:

    static bool contains(const struct rack::math::Rect& r, const Vec& pos)
    {
        return r.isContaining(pos);
    }
    using SvgWidget = SvgWidget;
//void SvgKnob::setSvg(std::shared_ptr<Svg> svg

    static void setSvg(SvgWidget* widget, std::shared_ptr<Svg> svg)
    {
        widget->setSvg(svg);
    }
    static void setSvg(SvgKnob* knob, std::shared_ptr<Svg> svg)
    {
        knob->setSvg(svg);
    }
    static std::shared_ptr<Svg> loadSvg(const char* path) 
    {
        return APP->window->loadSvg(
            SqHelper::assetPlugin(pluginInstance, path));
    }
    static void setPanel(ModuleWidget* widget, const char* path)
    {
         widget->setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, path)));
    }

    static void openBrowser(const char* url)
    {
        system::openBrowser(url);
    }
    static std::string assetPlugin(Plugin *plugin, const std::string& filename)
    {
        return asset::plugin(plugin, filename);
    } 
    static float engineGetSampleRate()
    {
        return APP->engine->getSampleRate();
    }
      static float engineGetSampleTime()
    {
        return APP->engine->getSampleTime();
    }

    template <typename T>
    static T* createParam(std::shared_ptr<IComposite> dummy, const Vec& pos, Module* module, int paramId )
    {
        return rack::createParam<T>(pos, module, paramId);
    }

    template <typename T>
    static T* createParamCentered(std::shared_ptr<IComposite> dummy, const Vec& pos, Module* module, int paramId )
    {
        return rack::createParamCentered<T>(pos, module, paramId);
    }

    static const NVGcolor COLOR_WHITE;
    static const NVGcolor COLOR_BLACK;

    static void setupParams(std::shared_ptr<IComposite> comp, Module* module)
    {
        const int n = comp->getNumParams();
        for (int i=0; i<n; ++i) {
            auto param = comp->getParam(i);
            module->params[i].config(param.min, param.max, param.def, param.name);
        }
    }

    static float getValue(ParamWidget* widget) {
        return (widget->paramQuantity) ?
            widget->paramQuantity->getValue() :
            0;
    }

    static void setValue(ParamWidget* widget, float v) {
        if (widget->paramQuantity) {
            widget->paramQuantity->setValue(v);
        }
    }
};

#else


class SqHelper
{
public:

    static std::string assetPlugin(Plugin *plugin, const std::string& filename)
    {
        return rack::assetPlugin(plugin, filename);
    } 
    static float engineGetSampleRate()
    {
        return rack::engineGetSampleRate();
    }

    static float engineGetSampleTime()
    {
        return rack::engineGetSampleTime();
    }
    static void openBrowser(const char* url)
    {
        rack::systemOpenBrowser(url);
    }

   static const NVGcolor COLOR_WHITE;
   static const NVGcolor COLOR_BLACK;

   template <typename T>
   static T* createParam(std::shared_ptr<IComposite> composite, const Vec& pos, Module* module, int paramId )
   {
       const auto data = composite->getParam(paramId);
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

    template <typename T>
    static T* createParamCentered(std::shared_ptr<IComposite> composite, const Vec& pos, Module* module, int paramId )
    {
        const auto data = composite->getParam(paramId);
        assert(data.min < data.max);
        assert(data.def >= data.min);
        assert(data.def <= data.max);
        return rack::createParamCentered<T>(
            pos,
            module, 
            paramId,
            data.min, data.max, data.def
        );
    }

    static float getValue(ParamWidget* widget) {
        return widget->value;
    }

    static void setValue(ParamWidget* widget, float v) {
        widget->setValue(v);
    }

    static void setPanel(ModuleWidget* widget, const char* path)
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = widget->box.size;
        panel->setBackground(SVG::load(SqHelper::assetPlugin(pluginInstance, path)));
        widget->addChild(panel);
    }

    static std::shared_ptr<SVG> loadSvg(const char* path) 
    {
        return SVG::load(
            SqHelper::assetPlugin(pluginInstance, path));
    }
};
#endif
