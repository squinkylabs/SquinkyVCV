#pragma once

/**
 * This is a collection of utilties that work on both VCV 1.0
 * and VCV 0.6.n
 */
#include "app.hpp"
#include "IComposite.h"

#ifdef __V1x

#include "engine/Module.hpp"
#include <string>

extern ::rack::plugin::Plugin *pluginInstance;
class SqHelper
{
public:

    static bool contains(const struct ::rack::math::Rect& r, const ::rack::math::Vec& pos)
    {
        return r.isContaining(pos);
    }
    using SvgWidget = ::rack::widget::SvgWidget;
    using SvgSwitch = ::rack::app::SvgSwitch;
    
    static void setSvg(SvgWidget* widget, std::shared_ptr<::rack::Svg> svg)
    {
        widget->setSvg(svg);
    }
    static void setSvg (::rack::app::SvgKnob* knob, std::shared_ptr<::rack::Svg> svg)
    {
        knob->setSvg(svg);
    }

    /**
     * loads SVG from plugin's assets,
     * unless pathIsAbsolute is ture
     */
    static std::shared_ptr<::rack::Svg> loadSvg(const char* path, bool pathIsAbsolute = false) 
    {
        if (pathIsAbsolute) {
            return ::rack::APP->window->loadSvg(path);
        } else {
            return ::rack::APP->window->loadSvg(
                SqHelper::assetPlugin(pluginInstance, path));
        }
    }

    static void setPanel(::rack::app::ModuleWidget* widget, const char* path)
    {
         widget->setPanel(::rack::APP->window->loadSvg(::rack::asset::plugin(pluginInstance, path)));
    }

    static void openBrowser(const char* url)
    {
        ::rack::system::openBrowser(url);
    }
    static std::string assetPlugin (::rack::plugin::Plugin *plugin, const std::string& filename)
    {
        return ::rack::asset::plugin(plugin, filename);
    } 
    static float engineGetSampleRate()
    {
        return ::rack::APP->engine->getSampleRate();
    }
    static float engineGetSampleTime()
    {
        return ::rack::APP->engine->getSampleTime();
    }

    template <typename T>
    static T* createParam(
        std::shared_ptr<IComposite> dummy, 
        const ::rack::math::Vec& pos, 
        ::rack::engine::Module* module, 
        int paramId )
    {
        return ::rack::createParam<T>(pos, module, paramId);
    }

    template <typename T>
    static T* createParamCentered(
        std::shared_ptr<IComposite> dummy, 
        const ::rack::math::Vec& pos, 
        ::rack::engine::Module* module,
        int paramId )
    {
        return ::rack::createParamCentered<T>(pos, module, paramId);
    }

    static const NVGcolor COLOR_WHITE;
    static const NVGcolor COLOR_BLACK;
    static const NVGcolor COLOR_SQUINKY;

    static void setupParams(
        std::shared_ptr<IComposite> comp,
        ::rack::engine::Module* module)
    {
        const int n = comp->getNumParams();
        for (int i=0; i<n; ++i) {
            auto param = comp->getParam(i);
            std::string paramName(param.name);
            // module->params[i].config(param.min, param.max, param.def, paramName);
            module->configParam(i, param.min, param.max, param.def, paramName);
        }
    }

    static float getValue (::rack::app::ParamWidget* widget) {
        return (widget->paramQuantity) ?
            widget->paramQuantity->getValue() :
            0;
    }

    static void setValue (::rack::app::ParamWidget* widget, float v) {
        if (widget->paramQuantity) {
            widget->paramQuantity->setValue(v);
        }
    }
};

#define DECLARE_MANUAL(TEXT, URL) void appendContextMenu(Menu *theMenu) override \
{ \
    ::rack::ui::MenuLabel *spacerLabel = new ::rack::ui::MenuLabel(); \
	theMenu->addChild(spacerLabel); \
    ManualMenuItem* manual = new ManualMenuItem(TEXT, URL); \
    theMenu->addChild(manual);   \
}

#else


class SqHelper
{
public:

    static std::string assetPlugin(Plugin *plugin, const std::string& filename)
    {
        return ::rack::assetPlugin(plugin, filename);
    } 
    static float engineGetSampleRate()
    {
        return ::rack::engineGetSampleRate();
    }

    static float engineGetSampleTime()
    {
        return ::rack::engineGetSampleTime();
    }
    static void openBrowser(const char* url)
    {
        ::rack::systemOpenBrowser(url);
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
       return ::rack::createParam<T>(
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
        return ::rack::createParamCentered<T>(
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


    using SvgWidget = SVGWidget;
    using SvgSwitch = SVGSwitch;
    static void setSvg(SvgWidget* widget, std::shared_ptr<SVG> svg)
    {
        widget->setSVG(svg);
    }
    static void setSvg(SVGKnob* knob, std::shared_ptr<SVG> svg)
    {
        knob->setSVG(svg);
    }

    static bool contains(struct Rect& r, const Vec& pos)
    {
        return r.contains(pos);
    }

};

#define DECLARE_MANUAL(TEXT, URL) Menu* createContextMenu() override \
{ \
    ::rack::ui::Menu* theMenu = ModuleWidget::createContextMenu(); \
    ManualMenuItem* manual = new ManualMenuItem(TEXT, URL); \
    theMenu->addChild(manual); \
    return theMenu; \
}

#endif
