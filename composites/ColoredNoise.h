#pragma once

#include <memory>
#include "assert.h"

#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"

template <class TBase>
class ColoredNoise : public TBase
{
public:
    ColoredNoise(struct Module * module) : TBase(module)
    {
        commonConstruct();
    }
    ColoredNoise() : TBase()
    {
        commonConstruct();
    }
    void setSampleRate(float rate)
    {
    }

    // must be called after setSampleRate
    void init()
    {
    }

    // Define all the enums here. This will let the tests and the widget access them.
    enum ParamIds
    {
        NUM_PARAMS
    };

    enum InputIds
    {
        NUM_INPUTS
    };

    enum OutputIds
    {
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    /**
    * Main processing entry point. Called every sample
    */
    void step();

    typedef float T;        // use floats for all signals
private:
    std::unique_ptr<ThreadClient> thread;
    void commonConstruct();
};


class NoiseServer : public ThreadServer
{
public:
    NoiseServer(std::shared_ptr<ThreadSharedState> state) : ThreadServer(state)
    {

    }
    virtual void handleMessage(ThreadMessage*)
    {
        assert(false);
    }
};

template <class TBase>
void ColoredNoise<TBase>::commonConstruct()
{
    std::shared_ptr<ThreadSharedState> threadState = std::make_shared<ThreadSharedState>();
    std::unique_ptr<ThreadServer> server(new NoiseServer(threadState));

    std::unique_ptr<ThreadClient> client(new ThreadClient(threadState, std::move(server)));
    this->thread = std::move(client);
}

template <class TBase>
void ColoredNoise<TBase>::step()
{

}