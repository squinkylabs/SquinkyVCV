#pragma once

#include "CommChannels.h"
/**
 * This class manages the communication between
 * misers and mixer expanders
 */
class MixerModule : public rack::engine::Module
{
public:
    MixerModule();

    void process(const ProcessArgs &args) override;
    virtual void internalProcess() = 0;
    virtual void requestModuleSolo(int) = 0;


    virtual bool amMaster() { return false; }

    #if 0
    virtual void setBusOutput(float*)
    {

    }
     virtual void setBusInput(float*)
    {
        
    }
    #endif

    /**
     * UI calls this to initiate a solo
     */
    void requestSolo(int channel);
protected:
    virtual void setExternalInput(const float*)=0;
    virtual void setExternalOutput(float*)=0;

private:
    /**
     * Expanders provide the buffers to talk to (send data to) the module to their right.
     * that module may be a master, or another expander. 
     * 
     * #1) Send data to right: use you own right producer buffer.
     * #2) Receive data from left:  use left's right consumer buffer
     * 
     * #3) Send data to the left: use your own left producer buffer.
     * #4) Receive data from right: user right's left consumer buffer 
     */
    float bufferFlipR[5];
    float bufferFlopR[5];
    float bufferFlipL[1];
    float bufferFlopL[1];

    CommChannelSend sendRightChannel;
    CommChannelSend sendLeftChannel;
    CommChannelReceive receiveRightChannel;
    CommChannelReceive receiveLeftChannel;

    /**
     * 0 - no request,
     * 1..4 is a request to solo channel n-1
     * 
     * Set by the the UI thread
     */
    int soloRequest = 0;

};

// Remember : "rightModule" is the module to your right
// producer and consumer are concepts of the engine, not us.
// rack will flip producer and consumer each process tick.
inline MixerModule::MixerModule()
{
    rightProducerMessage = bufferFlipR;
    rightConsumerMessage = bufferFlopR;
    leftProducerMessage = bufferFlipL;
    leftConsumerMessage = bufferFlopL;
}


// Each time we are called, we want the unit on the left to output
// all bus audio to the unit on its right.
// So - expander on the left will output to it's producer buffer.
// And - master on the right will get it's bus input from the consumer buffer 
//      from the unit on its right

inline void MixerModule::process(const ProcessArgs &args)
{

    // first, determine what modules are are paired with what
    // A Mix4 is not a master, and can pair with either a Mix4 or a MixM to the right
    const bool pairedRight = rightModule && 
        ((rightModule->model == modelMixMModule) || (rightModule->model == modelMix4Module)) &&
        !amMaster();

    // A MixM and a Mix4 can both pair with a Mix4 to the left
    const bool pairedLeft = leftModule &&
        (leftModule->model == modelMix4Module);

    //printf("\nmixer %p\n amMaster=%d, pairedLeft=%d right=%d\n", this, amMaster(), pairedLeft, pairedRight);
    //printf("rm=%d lm=%d\n", bool(rightModule), bool(leftModule));
    //fflush(stdout);

    assert(rightProducerMessage);
    assert(!pairedLeft || leftModule->rightConsumerMessage);

    // set a channel to send data to the right (case #1, above)
    setExternalOutput(pairedRight ? reinterpret_cast<float *>(rightProducerMessage) : nullptr);
    
    // set a channel to rx data from the left (case #2, above)
    setExternalInput(pairedLeft ? reinterpret_cast<float *>(leftModule->rightConsumerMessage) : nullptr);

   
    if (soloRequest) {
        // If solo requested, queue up reset solo commands for both sides     
        if (pairedRight) {
            printf("solo req, mod is paired R\n");
            sendRightChannel.send(CommCommand_ClearMute);
        }
        if (pairedLeft) {
            printf("solo req, mod is paired L\n");
            sendLeftChannel.send(CommCommand_ClearMute);
        }
      
        // tell our own module to solo
        requestModuleSolo(soloRequest);
        soloRequest = 0;
    }

    if (pairedRight) {
        // #1) Send data to right: use you own right producer buffer.
        uint32_t* outBuf = reinterpret_cast<uint32_t *>(rightProducerMessage);

        // #4) Receive data from right: user right's left consumer buffer
        const uint32_t* inBuf = reinterpret_cast<uint32_t *>(rightModule->leftConsumerMessage);
        sendRightChannel.go(outBuf + 4);
        uint32_t cmd = receiveRightChannel.rx(inBuf + 0);
        if (cmd == CommCommand_ClearMute) {
            printf("right read clearmute module=%p\n", this); fflush(stdout);
            requestModuleSolo(0);
            // now relay down to the left
            if (pairedLeft) {
                 sendLeftChannel.send(CommCommand_ClearMute);
            }
        }
  

    }
    if (pairedLeft) {
        // #3) Send data to the left: use your own left producer buffer.
        uint32_t* outBuf = reinterpret_cast<uint32_t *>(leftProducerMessage);
        
        // 2) Receive data from left:  use left's right consumer buffer
        const uint32_t* inBuf = reinterpret_cast<uint32_t *>(leftModule->rightConsumerMessage);
        sendLeftChannel.go(outBuf + 0);
        uint32_t cmd = receiveLeftChannel.rx(inBuf + 4);
        if (cmd == CommCommand_ClearMute) {
            printf("left read clear mute module %p\n", this); fflush(stdout);
            requestModuleSolo(0);
             // now relay down to the right
            if (pairedRight) {
                 sendRightChannel.send(CommCommand_ClearMute);
            }
        }
    }

    

    // Do the audio processing, and handle the left and right audio buses
    internalProcess();
}


inline void MixerModule::requestSolo(int channel)
{
    printf("\nUI req solo %d module=%p\n", channel, this); fflush(stdout);
    soloRequest = channel+1;      // Queue up a request for the audio thread.
                                // TODO: use atomic?
}
