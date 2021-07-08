
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
        DUMMY_PARAM,
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
        return std::make_shared<GMRDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */

    // TODO: process
    void step() override;

private:
    float reciprocalSampleRate = 0;
    std::shared_ptr<GenerativeTriggerGenerator2> gtg;
    GateTrigger inputClockProcessing;
    TriggerOutput outputProcessing;
};

template <class TBase>
inline void GMR2<TBase>::init() {
    StochasticGrammarDictionary::Grammar grammar = StochasticGrammarDictionary::getGrammar(0);
    gtg = std::make_shared<GenerativeTriggerGenerator>(
        AudioMath::random(),
        grammar.rules,
        grammar.numRules,
        grammar.firstRule);
}

template <class TBase>
inline void GMR2<TBase>::step() {
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
    //   const float numWaves = (float)Basic<TBase>::Waves::END;
    //   const float defWave = (float)Basic<TBase>::Waves::SIN;
    Config ret(0, 1, 0, "");
    switch (i) {
        case GMR2<TBase>::DUMMY_PARAM:
            ret = {0, 1, 0, "dummy"};
            break;
        default:
            assert(false);
    }
    return ret;
}