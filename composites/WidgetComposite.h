#pragma once

#ifdef __V1x
#include "engine/Module.hpp"
#include "engine/Port.hpp"
#include "engine/Engine.hpp"

using Input = rack::engine::Input;
using Output = rack::engine::Output;
using Param = rack::engine::Param;
using Light = rack::engine::Light;
using Module = rack::engine::Module;
#else

#include "engine.hpp"

using Input = rack::Input;
using Output = rack::Output;
using Param = rack::Param;
using Light = rack::Light;
using Module = rack::Module;
#endif

/**
 * Base class for composites embeddable in a VCV Widget
 * This is used for "real" implementations
 */
class WidgetComposite
{
public:
    WidgetComposite(Module * parent) :
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
#ifdef __V1x
        return rack::APP->engine->getSampleRate();
#else  
        return ::engineGetSampleRate();
#endif
    }
    
    float engineGetSampleTime()
    {
#ifdef __V1x
        return rack::APP->engine->getSampleTime();
#else  
        return ::engineGetSampleTime();
#endif
    }
protected:
    std::vector<Input>& inputs;
    std::vector<Output>& outputs;
    std::vector<Param>& params;
    std::vector<Light>& lights;
private:
};
