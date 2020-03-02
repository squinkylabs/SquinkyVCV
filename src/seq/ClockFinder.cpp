#include "ClockFinder.h"

#include "app.hpp"
#include "app/CableWidget.hpp"
#include "app/Scene.hpp"
#include "app/ModuleWidget.hpp"
#include "app/ParamWidget.hpp"
#include "app/PortWidget.hpp"
#include "engine/Engine.hpp"
#include "plugin/Model.hpp"
#include "widget/Widget.hpp"

#include "Seq.h"
#include "Seq4.h"
#include "WidgetComposite.h"

using ModuleWidget = ::rack::app::ModuleWidget;
using Model = ::rack::plugin::Model;
using PortWidget = ::rack::app::PortWidget;
using CableWidget = ::rack::app::CableWidget;
using ParamWidget = ::rack::app::ParamWidget;
using Engine = ::rack::engine::Engine;


/********************************* ALL STUFF ABOUT EXTERNAL CLOCKS **************/

/**
 * Descriptions of all the clocks we know about
 */
class ClockDescriptor 
{
public:
    std::string slug;
    int clockOutputIds[3];
    int clockRatioParamIds[3];          // for the three stand-alone, not counting master
    int runningParamId;
    //void dump() const;

    ClockDescriptor(const ClockDescriptor&)=delete;
    const ClockDescriptor& operator=(const ClockDescriptor&)=delete;
};

static const ClockDescriptor descriptors[] = {
    {"Clocked", 
        {1,2,3},
        {1,2,3},
        {13}
    },
    {"Clocked-Clkd", 
        {1,2,3},
        {0,1,2},
        {5}
    }
};

#if 0
void ClockDescriptor::dump() const
{
    assert((this == descriptors + 0) || (this == descriptors + 1)); 
    INFO("**** dump descriptor %p", this);
    INFO("    slug = %s", slug.c_str());
    INFO("    output ids = %d,%d,%d", clockOutputIds[0],clockOutputIds[1],clockOutputIds[2]);
    INFO(" runningParamID = %d", runningParamId);
}

static void dumpBoth()
{
    INFO("dump both");
    descriptors[0].dump();
    descriptors[1].dump();
    INFO("dump both done");

}
#endif

static const int numDescriptors = sizeof(descriptors) / sizeof(ClockDescriptor);

// class that knows about clocks from other manufacturers
class Clocks
{
public:
    using WidgetAndDescription = std::pair<ModuleWidget*, const ClockDescriptor* >;
   
    static WidgetAndDescription findClosestClocked(const ModuleWidget* fromSeq);

    /**
     * return.first will be a non null port widget to use for clock/
     * return.second will be true if the port is already patched
     */
    static std::pair<PortWidget*, bool> findBestClockOutput(WidgetAndDescription clocked);

    static std::vector<PortWidget*> findClockedOutputs(ModuleWidget* clocked, PortWidget* clock);

    static ParamWidget* getRatioParam(WidgetAndDescription clocked, int index);
    static ParamWidget* getRunningParam(WidgetAndDescription clocked);
private:
    using WidgetAndDescriptionS = std::vector<WidgetAndDescription>;
    static WidgetAndDescriptionS findClocks();

    /**
     * get the port for clock[index] (skipping over "master" clock)
     */
    static PortWidget* findClockOutput(WidgetAndDescription clock, int index);
};

Clocks::WidgetAndDescriptionS Clocks::findClocks()
{
    WidgetAndDescriptionS ret;
    auto rack = ::rack::appGet()->scene->rack;
    for (::rack::widget::Widget* w2 : rack->moduleContainer->children) {
        ModuleWidget* modwid = dynamic_cast<ModuleWidget *>(w2);
        if (modwid) {
            Model* model = modwid->model;
            for (int i=0; i< numDescriptors; ++i) {
                const ClockDescriptor* descriptor = descriptors + i;
                if (model->slug == descriptor->slug) {
                    ret.push_back( std::make_pair(modwid, descriptor));
                }
            }
        } else {
            WARN("was not a module widget");
        }
    }   
    return ret;
}

static double calcDistance(const ModuleWidget* a, const ModuleWidget* b)
{
    assert(a && b);
    auto rect = a->box.expand(b->box);
    const auto x = rect.size.x;
    const auto y = rect.size.y;
    return std::sqrt(x * x + y * y);
}

Clocks::WidgetAndDescription Clocks::findClosestClocked(const ModuleWidget* from)
{
    assert(from);
    WidgetAndDescriptionS clocks = findClocks();
    WidgetAndDescription ret(nullptr, nullptr);
    double closestDistance = 1000000000000000;
    for (auto clock : clocks) {
        double distance = calcDistance(clock.first, from);
        if (distance < closestDistance) {
            closestDistance = distance;
            ret = clock;
        }
    }
    return ret;
}

/**
 * return.first will be a non null port widget to use for clock/
 * return.second will be true if the port is already patched
 */
std::pair<PortWidget*, bool> Clocks::findBestClockOutput(WidgetAndDescription clock)
{
    for (int i=0; i<3; ++i) {
        auto port = findClockOutput(clock, i);
        auto cables = APP->scene->rack->getCablesOnPort(port);
        if (cables.empty()) {
            return std::make_pair(port, false);
        }
    }
    return std::make_pair(findClockOutput(clock, 0), true);
}

// index = 0 - clock 1 ... 2 - clock3 (skip master)
PortWidget* Clocks::findClockOutput(WidgetAndDescription clock, int index)
{
    const int targetPortId = clock.second->clockOutputIds[index];
    INFO("looking for port id %d slug %s", targetPortId, clock.second->slug.c_str());
    for (auto output : clock.first->outputs) {
        if ((output->portId) == targetPortId) {
            return output;
        }
    }
    assert(false);
    return nullptr;
}

// TODO: this uses hard coded output port numbers
std::vector<PortWidget*> Clocks::findClockedOutputs(ModuleWidget* clocked, PortWidget* clock) {
    int found = 1;
    std::vector<PortWidget*> ret(3);

    assert(clock);
    ret[0] = clock;
    for (auto output : clocked->outputs) {
        switch(output->portId) {
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

/**
 * Given a Clocked module and an index into one of the non-master clocks,
 * Find it and return the param widget for the ratio knob.
 */
ParamWidget* Clocks::getRatioParam(WidgetAndDescription clocked, int _index)
{
    const int clockRatioParamId = clocked.second->clockRatioParamIds[_index];
    for (auto param : clocked.first->params) {
        if (!param->paramQuantity) {
            WARN("param has no quantity");
            return nullptr;
        }
        const int id = param->paramQuantity->paramId;
        if (id == clockRatioParamId) {
            return param;
        }
    }
    assert(false);
    return nullptr;
}

class Seqs
{
public:
    static std::vector<PortWidget*> findInputs(const ModuleWidget* seq);
    static bool anyConnected(const std::vector<PortWidget*>& ports);
    static float clockDivToClockedParam(int div);
    static ParamWidget* getRunningParam(ModuleWidget*, bool isSeqPlusPlus);
};

static ParamWidget* findParamWidgetForParamId(ModuleWidget* moduleWidget, int paramID)
{
    assert(paramID >= 0);
    assert(paramID < 20);
    for (auto param : moduleWidget->params) {
        if (!param->paramQuantity) {
            WARN("param has no quantity");
            return nullptr;
        }
        const int id = param->paramQuantity->paramId;
        if (id == paramID) {
            return param;
        }
    }
    assert(false);
    return nullptr;
}

ParamWidget*  Clocks::getRunningParam(WidgetAndDescription clocked)
{
    const int paramID = clocked.second->runningParamId;
    return findParamWidgetForParamId(clocked.first, paramID);
}

ParamWidget* Seqs::getRunningParam(ModuleWidget* seqWidget, bool isSeqPlusPlus)
{
    
    const int paramID = isSeqPlusPlus ?  int(Seq<WidgetComposite>::RUNNING_PARAM) : int(Seq4<WidgetComposite>::RUNNING_PARAM);
    return findParamWidgetForParamId(seqWidget, paramID);  
}

// TODO: generalize to both seq
// clock, run, reset

std::vector<PortWidget*> Seqs::findInputs(const ModuleWidget* seq) {
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

bool Seqs::anyConnected(const std::vector<PortWidget*>& ports)
{
    for (auto port : ports) {
        auto cables = APP->scene->rack->getCablesOnPort(port);
        if (!cables.empty()) {
            return true;
        }
    }
    return false;
}


// TODO: get this from descriptor - don't assume ratios
float Seqs::clockDivToClockedParam(int div)
{
    float ret = 0;
    assert(div > 0);
    if (div <= 16) {
        ret = div + 1;
    } else if (div == 32) {
        ret = 7+17;
    } else if (div == 64) {
        ret = 7 + 17 + 9;
    } else {
        assert(false);
    }
    return ret;
}


static void setParamOnWidget(ModuleWidget* moduleWidget, ParamWidget* paramWidget, float value)
{
    auto module = moduleWidget->module;
    const int paramId = paramWidget->paramQuantity->paramId;
    APP->engine->setParam(module, paramId, value);
}

void ClockFinder::go(ModuleWidget* host, int div, int clockInput, int runInput, int resetInput)
{
    const bool isSeqPlusPlus = true;

    // we are hard coded to these values. If module changes, we will have to get smarter/
    assert(clockInput == 0);
    assert(runInput == 2);
    assert(resetInput == 1);

    auto moduleAndDescription = Clocks::findClosestClocked(host);
    if (!moduleAndDescription.first) {
        return;
    }

    auto clockOutput = Clocks::findBestClockOutput(moduleAndDescription);
    assert(clockOutput.first != nullptr);

    auto outputs = Clocks::findClockedOutputs(moduleAndDescription.first, clockOutput.first);
    assert(outputs.size() == 3);

    auto inputs = Seqs::findInputs(host);
    assert(inputs.size() == 3);

    if (outputs.size() != 3 || inputs.size() != 3) {
        WARN("bad I/O matchup. o=%d, i=%d", outputs.size(), inputs.size());
        return;
    }

    if (Seqs::anyConnected(inputs)) {
        return;
    }

    // const NVGcolor color = nvgRGB(0, 0, 0);
    for (int i=0; i<3; ++i) {
        CableWidget* cable = new CableWidget();
        // cable->color = color;
        cable->setOutput(outputs[i]);
        cable->setInput(inputs[i]);
        APP->scene->rack->addCable(cable);
    }

    // something in this block is crashing

    if (!clockOutput.second) {
        // if the clock output was empty before us, then we can set the
        // ratio to match the sequencer.
        const int clockIndex = clockOutput.first->portId - 1;
        auto paramWidget = Clocks::getRatioParam(moduleAndDescription, clockIndex);
        if (paramWidget) {
            const float value = Seqs::clockDivToClockedParam(div);
            setParamOnWidget(moduleAndDescription.first, paramWidget, value);
        }
    }

    // now make everyone running = false;
    {
        auto clockParamWidget = Clocks::getRunningParam(moduleAndDescription);
        auto seqParamWidget = Seqs::getRunningParam(host, isSeqPlusPlus);
        assert(clockParamWidget && seqParamWidget);
        if (seqParamWidget) {
            setParamOnWidget(host, seqParamWidget, 0);
        }
        if (clockParamWidget) {
            setParamOnWidget(moduleAndDescription.first, clockParamWidget, 1);
        }
    }
}


