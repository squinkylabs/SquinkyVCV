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
        SLOPE_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        NUM_INPUTS
    };

    enum OutputIds
    {
        AUDIO_OUTPUT,
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

    float getSlope() const;

    int _msgCount() const;  // just for debugging

    typedef float T;        // use floats for all signals
private:

    bool isRequestPending = false;

    /**
     * The FFT frame we are playing from.
     */
    NoiseMessage* curData = nullptr;

    /**
     * The "play head" in curData where we will get next sample
     */
    int curDataOffset = 0;

    int messageCount = 0; 

    std::unique_ptr<ThreadClient> thread;
    ManagedPool<NoiseMessage, 2> messagePool;

    void serviceFFTServer();
    void serviceAudio();
    void serviceInputs();
    void commonConstruct();
};


// TODO: may not be possible to have a zero arg ctor for managed Pool to use...
class NoiseMessage : public ThreadMessage
{
public:

    NoiseMessage() : ThreadMessage(Type::NOISE),
        dataBuffer( new FFTDataReal(defaultNumBins))
    {
    }

    NoiseMessage(int numBins) : ThreadMessage(Type::NOISE),
        dataBuffer(new FFTDataReal(numBins))
    {
    }
    ~NoiseMessage()
    {
    }
    const int defaultNumBins = 64 * 1024;

    ColoredNoiseSpec noiseSpec;

    /** Server is going to fill this buffer up with time-domain data
     */
    //FFTDataReal* const dataBuffer=nullptr;  // TODO:delete
    std::unique_ptr<FFTDataReal> dataBuffer;
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
        //printf("server got message\n");
        if (msg->type != ThreadMessage::Type::NOISE) {
            assert(false);
            return;
        }

        // Unpack the parameters, convert to frequency domain "noise" recipe
        NoiseMessage* noiseMessage = static_cast<NoiseMessage*>(msg);
        reallocSpectrum(noiseMessage);
        FFT::makeNoiseSpectrum(noiseSpectrum.get(),
                              noiseMessage->noiseSpec);

        // Now inverse FFT to time domain noise in client's buffer
        FFT::inverse(noiseMessage->dataBuffer.get(), *noiseSpectrum.get());
        FFT::normalize(noiseMessage->dataBuffer.get());
       // printf("server sending message back to client\n");
        sendMessageToClient(noiseMessage);
      //  printf("sent\n");
    }
private:
    std::unique_ptr<FFTDataCpx> noiseSpectrum;

    // may do nothing, may create the first buffer,
    // may delete the old buffer and make a new one.
    void reallocSpectrum(const NoiseMessage* msg)
    {
        if (noiseSpectrum && ((int)noiseSpectrum->size() == msg->dataBuffer->size())) {
            return;
        }

        noiseSpectrum.reset(new FFTDataCpx(msg->dataBuffer->size()));
    }
};

template <class TBase>
float ColoredNoise<TBase>::getSlope() const
 { 
     // TODO: atomic?
     return curData ? curData->noiseSpec.slope : 0;
}

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
void ColoredNoise<TBase>::serviceFFTServer()
{
    // do we need to ask for more data?
   // bool requestPending = false;
   // NoiseMessage* curData = nullptr;

    // see if we need to request first frame of sample data
    if (!isRequestPending && !curData) {
       // printf("try making first request\n");
        NoiseMessage* msg = messagePool.pop();
       
        bool sent = thread->sendMessage(msg);
        if (sent) {
          //  printf("made first request\n");
            isRequestPending = true;
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
        curDataOffset = 0;
        isRequestPending = false;
    }
}

template <class TBase>
void ColoredNoise<TBase>::serviceAudio()
{
    float output = 0;
    if (curData) {
        output = curData->dataBuffer->get(curDataOffset++);
        if (curDataOffset >= (int)curData->dataBuffer->size())
        {
            curDataOffset = 0;
        }
    }

    TBase::outputs[AUDIO_OUTPUT].value = output;
}


template <class TBase>
void ColoredNoise<TBase>::serviceInputs()
{

    if (isRequestPending) {
        return;     // can't do anything until server is free.
    }
    if (!curData) {
        return;     // if we don't have data, we will be asking anyway
    }
   
    // get slope input to one decimal place
    float x = TBase::params[SLOPE_PARAM].value;
    int i = int(x * 10);
    x = i / 10.f;
    ColoredNoiseSpec sp;
    sp.slope = x;
    if (!(sp != curData->noiseSpec)) {
        return;
    }
    printf("new noise spec\n");

    NoiseMessage* msg = messagePool.pop();
    assert(msg);
    if (!msg) {
        return;
    }
    msg->noiseSpec = sp;
    // TODO: put this logic in one place
    bool sent = thread->sendMessage(msg);
    if (sent) {
          printf("made additional request\n");
        isRequestPending = true;
    } else {
        printf("send failed\n");
    }

}

template <class TBase>
void ColoredNoise<TBase>::step()
{
    serviceFFTServer();
    serviceAudio();
    serviceInputs();
}