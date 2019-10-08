#pragma once

/**
 * This is a collection of utilties that work on both VCV 1.0
 * and VCV 0.6.n
 */
#include "app.hpp"
#include "IComposite.h"

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
            return ::rack::appGet()->window->loadSvg(path);
        } else {
            return ::rack::appGet()->window->loadSvg(
                SqHelper::assetPlugin(pluginInstance, path));
        }
    }

    static void setPanel(::rack::app::ModuleWidget* widget, const char* path)
    {
         widget->setPanel(::rack::appGet()->window->loadSvg(::rack::asset::plugin(pluginInstance, path)));
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
        return ::rack::appGet()->engine->getSampleRate();
    }
    static float engineGetSampleTime()
    {
        return ::rack::appGet()->engine->getSampleTime();
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
