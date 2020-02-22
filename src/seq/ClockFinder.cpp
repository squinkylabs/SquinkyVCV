#include "ClockFinder.h"

#include "app.hpp"
#include "app/Scene.hpp"
#include "app/ModuleWidget.hpp"
#include "plugin/Model.hpp"
#include "widget/Widget.hpp"

using ModuleWidget = ::rack::app::ModuleWidget;
using Model = ::rack::plugin::Model;


static ModuleWidget* findClocked()
{
    const std::string ck("Clocked");
    auto rack = ::rack::appGet()->scene->rack;
    for (::rack::widget::Widget* w2 : rack->moduleContainer->children) {
        ModuleWidget* modwid = dynamic_cast<ModuleWidget *>(w2);
        if (modwid) {
            Model* model = modwid->model;
            INFO("******");
            INFO("name = %s, slug = %s", model->name.c_str(), model->slug.c_str());
            
            if (model->slug == ck) {
                return modwid;
            }
        } else {
            WARN("was not a module widget");
        }
    }   
    return nullptr;
}

void ClockFinder::go(ModuleWidget* host)
{
    INFO("Clock Finder");
    ModuleWidget* modwid = findClocked();
    if (modwid) {

    }
}