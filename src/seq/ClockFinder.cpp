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
                ret[2] = input;
                ++found;
                break;
            case 2:
                ret[1] = input;
                ++found;
                break;
         }
     }
     return (found == 3 ) ? ret : std::vector<PortWidget*>();
}

// Since we already know which one to use for clock, we pass that int
static std::vector<PortWidget*> findClockedOutputs(ModuleWidget* clocked, PortWidget* clock) {
    int found = 1;
    std::vector<PortWidget*> ret(3);

    assert(clock);
    ret[0] = clock;
    for (auto output : clocked->outputs) {
        switch(output->portId) {
    #if 0
            case 1:         // clock 1
                ret[0] = output;
                ++found;
                break;
    #endif
            case 5:         // run
                ret[1] = output;
                ++found;
                break;
            case 4:         // reset
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

// index = 0 - clock 1 ... 2 - clock3 (skip master)
PortWidget* findClockOutput(ModuleWidget* clocked, int index)
{
    for (auto output : clocked->outputs) {
        if ((output->portId) == (index + 1)) {
            return output;
        }
    }
    assert(false);
    return nullptr;
}

/**
 * return.first will be a non null port widget to use for clock/
 * return.second will be true if the port is already patched
 */
static std::pair<PortWidget*, bool> findBestClockOutput(ModuleWidget* clocked)
{
    for (int i=0; i<3; ++i) {
        auto port = findClockOutput(clocked, i);
        auto cables = APP->scene->rack->getCablesOnPort(port);
        if (cables.empty()) {
            return std::make_pair(port, false);
        }
    }
     return std::make_pair(findClockOutput(clocked, 0), true);
}

void ClockFinder::go(ModuleWidget* host)
{
    INFO("Clock Finder");
    ModuleWidget* clockedModule = findClocked();
    if (!clockedModule) {
        return;
    }

    auto clockOutput = findBestClockOutput(clockedModule);
    assert(clockOutput.first != nullptr);

    auto outputs = findClockedOutputs(clockedModule, clockOutput.first);
    auto inputs = findSeqInputs(host);
    if (outputs.size() != 3 || inputs.size() != 3) {

        WARN("bad I/O matchup. o=%d, i=%d", outputs.size(), inputs.size());
        return;
    }

    if (anyConnected(inputs)) {
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