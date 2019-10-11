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
class WidgetComposite
{
public:

    using Port = ::rack::engine::Port;
    
    WidgetComposite(::rack::engine::Module * parent) :
        inputs(parent->inputs),
        outputs(parent->outputs),
        params(parent->params),
        lights(parent->lights)
    {
    }
    virtual ~WidgetComposite() {}
    virtual void step()
    {
    };
    float engineGetSampleRate()
    {
        return ::rack::appGet()->engine->getSampleRate();
    }
    
    float engineGetSampleTime()
    {
        return ::rack::appGet()->engine->getSampleTime();
    }
protected:
    std::vector<Input>& inputs;
    std::vector<Output>& outputs;
    std::vector<Param>& params;
    std::vector<Light>& lights;
private:
};
