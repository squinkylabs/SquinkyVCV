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

// For now, let's hard code for one expander
inline void MixerModule::process(const ProcessArgs &args)
{
#if 0
     if (rightModule) {
        printf("\n%p I have a right module ammaster=%d\n", this, amMaster()); 
        printf("lpm=%p rpm = %p\n", 
            leftProducerMessage, 
            rightProducerMessage);
        printf("lcm=%p rcm = %p\n", 
            leftConsumerMessage, 
            rightConsumerMessage);
        printf("this flip=%p, flop=%p\n", bufferFlip, bufferFlop);
        fflush(stdout);
    }
#endif

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
    fflush(stdout);


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