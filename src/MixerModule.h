#pragma once

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

    // for debugging
    virtual bool amMaster() { return false; }
    virtual void setBusOutput(float*)
    {

    }
     virtual void setBusInput(float*)
    {
        
    }
protected:
    virtual void setExternalInput(const float*)=0;
    virtual void setExternalOutput(float*)=0;
private:
   // float lConsumerBuffer[8];
   // float lProducerBuffer[8];
   // float rConsumerBuffer[8];
   // float rProducerBuffer[8];

    /**
     * Expanders provide the buffers to talk to the module to their right.
     * that module may be a master, or another expander.
     */
    float bufferFlip[8];
    float bufferFlop[8];
};

// Remember : "rightModule" is the module to your right
// producer and consumer and concepts of the engine, not us.
// rack will flip producer and consumer each
inline MixerModule::MixerModule()
{
    rightProducerMessage = bufferFlip;
    rightConsumerMessage = bufferFlop;
}


// Each time we are called, we want the unit on the left to output
// all bus audio to the unit on its right.
// So - expander on the left will output to it's producer buffer.
// And - master on the right will get it's bus input from the consumer buffer 
//      from the unit on its right

inline void MixerModule::process(const ProcessArgs &args)
{
   #if 1
    // first, determine what modules are are paired with what
    // A Mix4 is not a master, and can pair with either a Mix4 or a MixM to the right
    const bool pairedRight = rightModule && 
        ((rightModule->model == modelMixMModule) || (rightModule->model == modelMix4Module)) &&
        !amMaster();

    // A MixM and a Mix4 can both pair with a Mix4 to the left
    const bool pairedLeft = leftModule &&
        (leftModule->model == modelMix4Module);

    //printf("\nmixer %p\n amMaster=%d, pairedLeft=%d right=%d\n", this, amMaster(), pairedLeft, pairedRight);
    #else 
    bool pairedLeft = false;
    bool pairedRight = false;
    #endif
    //printf("rm=%d lm=%d\n", bool(rightModule), bool(leftModule));
    //fflush(stdout);

    assert(rightProducerMessage);
    assert(!pairedLeft || leftModule->rightConsumerMessage);

    // "our" mixer will send stuff out using our out right message buffer.
    // (we don't have a left one, btw)
    setExternalOutput(pairedRight ? reinterpret_cast<float *>(rightProducerMessage) : nullptr);
    setExternalInput(pairedLeft ? reinterpret_cast<float *>(leftModule->rightConsumerMessage) : nullptr);

    internalProcess();

}


#if 0
inline void MixerModule::process(const ProcessArgs &args)
{
#if 1
static int x =0;
if (++x == 50) {
     if (rightModule) {
        printf("\n\n %p I have a right module ammaster=%d\n", this, amMaster()); 
        printf("lpm=%p rpm = %p\n", 
            leftProducerMessage, 
            rightProducerMessage);
        printf("lcm=%p rcm = %p\n", 
            leftConsumerMessage, 
            rightConsumerMessage);
        printf("this flip=%p, flop=%p\n", bufferFlip, bufferFlop);
        fflush(stdout);
    }
}
#endif

#if 0
    if (rightModule) {
        printf("right module model = %p, mixM = %p\n",
            rightModule->model,
            modelMixMModule);
    }
     if (leftModule) {
        printf("left module model = %p, mix4 = %p\n",
            leftModule->model,
            modelMix4Module);
    }
    #endif



#if 0
    // set up the audio busses buffers for this processing cycle
    if (!amMaster()) {
        // slave module
        // must send my output to this->rightProducerBuffer

        setBusOutput( static_cast<float *>(this->rightProducerMessage));
    } else {
        // master module.
        // must get my input from left->rightConsumerBuffer
        if (leftModule) {
            setBusInput(static_cast<float *>(leftModule->rightConsumerMessage));
        }
    }
    #endif


    // Now do the real mixer processing
    internalProcess();
}
#endif