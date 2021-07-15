
#pragma once

#include <memory>

#include "GenerativeTriggerGenerator2.h"
#include "IComposite.h"
#include "ObjectCache.h"
#include "TriggerOutput.h"

namespace rack {
namespace engine {
struct Module;
}
}  // namespace rack
using Module = ::rack::engine::Module;

template <class TBase>
class GMR2Description : public IComposite {
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 */
template <class TBase>
class GMR2 : public TBase {
public:
    GMR2(Module* module) : TBase(module), inputClockProcessing(true) {
    }
    GMR2() : TBase(), inputClockProcessing(true) {
    }
    void setSampleRate(float rate) {
        reciprocalSampleRate = 1 / rate;
    }

    // must be called after setSampleRate
    void init();

    enum ParamIds {
        DUMMY_PARAM1,
        DUMMY_PARAM2,
        DUMMY_PARAM3,
        DUMMY_PARAM4,
        DUMMY_PARAM5,
        DUMMY_PARAM6,
        NUM_PARAMS
    };

    enum InputIds {
        CLOCK_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        TRIGGER_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription() {
        return std::make_shared<GMR2Description<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */

    // TODO: process
    //void step() override;
     void process(const typename TBase::ProcessArgs& args) override;

private:
    float reciprocalSampleRate = 0;
    std::shared_ptr<GenerativeTriggerGenerator2> gtg;
    GateTrigger inputClockProcessing;
    TriggerOutput outputProcessing;
};

template <class TBase>
inline void GMR2<TBase>::init() {
    // StochasticGrammarDictionary::Grammar grammar = StochasticGrammarDictionary::getGrammar(0);
    StochasticGrammarPtr grammar = StochasticGrammar::getDemoGrammar(StochasticGrammar::DemoGrammar::demo);
    gtg = std::make_shared<GenerativeTriggerGenerator2>(
        AudioMath::random(),
        grammar);
}

template <class TBase>
inline void GMR2<TBase>::process(const typename TBase::ProcessArgs& args)  {
    bool outClock = false;
    float inClock = TBase::inputs[CLOCK_INPUT].getVoltage(0);
    inputClockProcessing.go(inClock);
    if (inputClockProcessing.trigger()) {
        outClock = gtg->clock();
    }
    outputProcessing.go(outClock);
    TBase::outputs[TRIGGER_OUTPUT].setVoltage(outputProcessing.get(), 0);
}

template <class TBase>
int GMR2Description<TBase>::getNumParams() {
    return GMR2<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config GMR2Description<TBase>::getParam(int i) {
    Config ret(0, 1, 0, "");
    switch (i) {
        case GMR2<TBase>::DUMMY_PARAM1:
            ret = {0, 1, 0, "dummy1"};
            break;
        case GMR2<TBase>::DUMMY_PARAM2:
            ret = {0, 1, 0, "dummy2"};
            break;
        case GMR2<TBase>::DUMMY_PARAM3:
            ret = {0, 1, 0, "dummy3"};
            break;
        case GMR2<TBase>::DUMMY_PARAM4:
            ret = {0, 1, 0, "dummy4"};
            break;
        case GMR2<TBase>::DUMMY_PARAM5:
            ret = {0, 1, 0, "dummy5"};
            break;
        case GMR2<TBase>::DUMMY_PARAM6:
            ret = {0, 1, 0, "dummy6"};
            break;
        default:
            assert(false);
    }
    return ret;
}