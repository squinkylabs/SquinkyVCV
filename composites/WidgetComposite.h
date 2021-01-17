#pragma once

#include "rack.hpp"

using Input = ::rack::engine::Input;
using Output = ::rack::engine::Output;
using Param = ::rack::engine::Param;
using Light = ::rack::engine::Light;
using Module = ::rack::engine::Module;

/**
 * Base class for composites embedable in a VCV Widget
 * This is used for "real" implementations
 */
class WidgetComposite {
public:
    using Port = ::rack::engine::Port;
    using ProcessArgs = ::rack::engine::Module::ProcessArgs;

    WidgetComposite(::rack::engine::Module* parent) : inputs(parent->inputs),
                                                      outputs(parent->outputs),
                                                      params(parent->params),
                                                      lights(parent->lights) {
    }
    virtual ~WidgetComposite() {}
    WidgetComposite(const WidgetComposite&) = delete;
    WidgetComposite& operator=(const WidgetComposite&) = delete;

    virtual void step(){};
    virtual void process(const ProcessArgs& args) {
    }
    float engineGetSampleRate()

    {
        return ::rack::appGet()->engine->getSampleRate();
    }

    float engineGetSampleTime() {
        return ::rack::appGet()->engine->getSampleTime();
    }

    virtual void onSampleRateChange() {
    }

protected:
    // These are references that point to the parent (real ones).
    // They are connected in the ctor
    std::vector<Input>& inputs;
    std::vector<Output>& outputs;
    std::vector<Param>& params;
    std::vector<Light>& lights;

private:
};
