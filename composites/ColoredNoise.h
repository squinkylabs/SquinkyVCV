#pragma once

#include <memory>
#include "assert.h"

#include "ManagedPool.h"
#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"

#include "FFTData.h"
#include "FFT.h"

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
    ~ColoredNoise()
    {
        thread.reset();     // kill the threads before deleting other things
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

    int _msgCount() const;  // just for debugging

    typedef float T;        // use floats for all signals
private:
    bool requestPending = false;
    NoiseMessage* curData = nullptr;
    int messageCount = 0;

    std::unique_ptr<ThreadClient> thread;
    void commonConstruct();

 
    ManagedPool<NoiseMessage, 2> messagePool;
};


// TODO: may not be possible to have a zero arg ctor for managed Pool to use...
class NoiseMessage : public ThreadMessage
{
public:
    NoiseMessage() : ThreadMessage(Type::NOISE),
        dataBuffer(new FFTDataReal(numBins))
    {
       
    }
    ~NoiseMessage()
    {
      //  assert(false);  // we are getting destroyed while thread still running
    }
    const int numBins = 64 * 1024;

    float noiseSlope=0;
    float highFrequencyCorner=0;
    float sampleRate=44100;

    /** Server is going to fill this buffer up with time-domain data
     */
    FFTDataReal* const dataBuffer=nullptr;  // TODO:delete
};

class NoiseServer : public ThreadServer
{
public:
    NoiseServer(std::shared_ptr<ThreadSharedState> state) : ThreadServer(state)
    {

    }
protected:
    /**
     * This is called on the server thread, not the audio thread.
     * We have plenty of time to do some heavy lifting here.
     */
    virtual void handleMessage(ThreadMessage* msg) override
    {
        printf("server got message\n");
        if (msg->type != ThreadMessage::Type::NOISE) {
            assert(false);
            return;
        }

        // Unpack the parameters, convert to frequency domain "noise" recipe
        NoiseMessage* noiseMessage = static_cast<NoiseMessage*>(msg);
        reallocRecipe(noiseMessage);
        FFT::makeNoiseFormula(noiseRecipe.get(),
                              noiseMessage->noiseSlope,
                              noiseMessage->highFrequencyCorner,
                              noiseMessage->sampleRate);

        // Now inverse FFT to time domain noise in client's buffer
        FFT::inverse(noiseMessage->dataBuffer, *noiseRecipe.get());
        printf("server sending message back to client\n");
        sendMessageToClient(noiseMessage);
        printf("sent\n");
    }
private:
    std::unique_ptr<FFTDataCpx> noiseRecipe;

    // may do nothing, may create the first buffer,
    // may delete the old buffer and make a new one.
    void reallocRecipe(const NoiseMessage* msg)
    {
        if (noiseRecipe && (noiseRecipe->size() == msg->numBins)) {
            return;
        }

        noiseRecipe.reset(new FFTDataCpx(msg->numBins));
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
int ColoredNoise<TBase>::_msgCount() const
{
    return messageCount;
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
        ++messageCount;
        assert(newMsg->type == ThreadMessage::Type::NOISE);
        NoiseMessage* noise = static_cast<NoiseMessage*>(newMsg);
        
        // give the last one back
        if (curData) {
            messagePool.push(curData);
        }
        curData = noise;
    }
}