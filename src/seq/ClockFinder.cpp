#include "ClockFinder.h"

#include "app.hpp"
#include "app/CableWidget.hpp"
#include "app/Scene.hpp"
#include "app/ModuleWidget.hpp"
#include "app/PortWidget.hpp"
#include "plugin/Model.hpp"
#include "widget/Widget.hpp"

using ModuleWidget = ::rack::app::ModuleWidget;
using Model = ::rack::plugin::Model;
using PortWidget = ::rack::app::PortWidget;
using CableWidget = ::rack::app::CableWidget;

static ModuleWidget* findClocked()
{
    const std::string ck("Clocked");
    auto rack = ::rack::appGet()->scene->rack;
    for (::rack::widget::Widget* w2 : rack->moduleContainer->children) {
        ModuleWidget* modwid = dynamic_cast<ModuleWidget *>(w2);
        if (modwid) {
            Model* model = modwid->model;
            if (model->slug == ck) {
                return modwid;
            }
        } else {
            WARN("was not a module widget");
        }
    }   
    return nullptr;
}

// TODO: generalize to both seq
// clock, run, reset
static std::vector<PortWidget*> findSeqInputs(ModuleWidget* seq) {
    int found = 0;
     std::vector<PortWidget*> ret(3);

     for (auto input : seq->inputs) {
         switch(input->portId) {
            case 0:
                ret[0] = input;
                ++found;
                break;
            case 1:
                ret[1] = input;
                ++found;
                break;
            case 2:
                ret[2] = input;
                ++found;
                break;
         }
     }
     return (found == 3 ) ? ret : std::vector<PortWidget*>();
}

static std::vector<PortWidget*> findClockedOutputs(ModuleWidget* clocked) {
    int found = 0;
     std::vector<PortWidget*> ret(3);

     for (auto output : clocked->outputs) {
         switch(output->portId) {
            case 1:
                ret[0] = output;
                ++found;
                break;
            case 5:
                ret[1] = output;
                ++found;
                break;
            case 4:
                ret[2] = output;
                ++found;
                break;
         }
     }
     return (found == 3 ) ? ret : std::vector<PortWidget*>();
}

static bool anyConnected(const std::vector<PortWidget*>& ports)
{
    for (auto port : ports) {
        auto cables = APP->scene->rack->getCablesOnPort(port);
        if (!cables.empty()) {
            return true;
        }
    }
    return false;
}

void ClockFinder::go(ModuleWidget* host)
{
    INFO("Clock Finder");
    ModuleWidget* modwid = findClocked();
    if (!modwid) {
        return;
    }

    auto outputs = findClockedOutputs(modwid);
    auto inputs = findSeqInputs(host);
    if (outputs.size() != 3 || inputs.size() != 3) {
        return;
    }

    if (anyConnected(inputs) || anyConnected(outputs)) {
        WARN("not patching, some are already patched\n");
        return;
    }
 
    const NVGcolor color = nvgRGB(0, 0, 0);
    for (int i=0; i<3; ++i) {
        CableWidget* cable = new CableWidget();
            cable->color = color;
        cable->setOutput(outputs[i]);
        cable->setInput(inputs[i]);
        APP->scene->rack->addCable(cable);
    }
    INFO("done patching");
}