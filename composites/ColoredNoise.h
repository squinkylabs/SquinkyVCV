#pragma once

#include <memory>
#include "assert.h"

#include "ManagedPool.h"
#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"

#include "FFTData.h"

class NoiseMessage;

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
    bool requestPending = false;
    NoiseMessage* curData = nullptr;

    std::unique_ptr<ThreadClient> thread;
    void commonConstruct();

 
    ManagedPool<NoiseMessage, 2> messagePool;
};


class NoiseServer : public ThreadServer
{
public:
    NoiseServer(std::shared_ptr<ThreadSharedState> state) : ThreadServer(state)
    {

    }
    virtual void handleMessage(ThreadMessage*) override
    {
        assert(false);
    }
};

// TODO: may not be possible to have a zero arg ctor for managed Pool to use...
class NoiseMessage : public ThreadMessage
{
public:
    NoiseMessage() : ThreadMessage(Type::NOISE)
    {
        dataBuffer = new FFTDataCpx(numBins);
    }
    const int numBins = 64 * 1024;
    //  static FFTDataCpx* makeNoiseFormula(float slope, float highFreqCorner, int frameSize);
    float noiseSlope;
    float highFrequencyCorner;
    float sampleRate;
    FFTDataCpx* dataBuffer;

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
    // do we need to ask for more data?
   // bool requestPending = false;
   // NoiseMessage* curData = nullptr;

    // see if we need to request first frame of sample data
    if (!requestPending && !curData) {
        printf("try making first request\n");
        NoiseMessage* msg = messagePool.pop();
       
        bool sent = thread->sendMessage(msg);
        if (sent) {
            printf("made first request\n");
            requestPending = true;
        }
    }

    // see if any messages came back for us
    ThreadMessage* newMsg = thread->getMessage();
    if (newMsg) {
        assert(false);      // finish me
    }
}