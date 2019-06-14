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

    /**
     * Concrete subclass implements this do accept a request
     * for solo change.
     */
    virtual void requestModuleSolo(SoloCommands) = 0;


    /**
     * Master module will override this to return true.
     * Let's us (base class) know if we are master or slave.
     */
    virtual bool amMaster() { return false; }

    /**
     * UI calls this to initiate a solo. UI
     * will only call with  SoloCommands::SOLO_0..3
     * (for now)
     */
    void requestSoloFromUI(SoloCommands);
protected:

    /**
     * Concrete subclass overrides these to transfer audio
     * with neighbors. subclass will fill input and consume output
     * on its process call.
     */
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

    // This gets set when UI calls us ask us to do something
    SoloCommands soloRequestFromUI = SoloCommands::DO_NOTHING;

    // This is set by us after executing a request from UI.
    // We use it to track our state
    SoloCommands currentSoloStatusFromUI = SoloCommands::DO_NOTHING;
};

// Remember : "rightModule" is the module to your right
// producer and consumer are concepts of the engine, not us.
// rack will flip producer and consumer each process tick.
inline MixerModule::MixerModule()
{
    rightExpander.producerMessage = bufferFlipR;
    rightExpander.consumerMessage = bufferFlopR;
    leftExpander.producerMessage = bufferFlipL;
    leftExpander.consumerMessage = bufferFlopL;
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
    const bool pairedRight = rightExpander.module && 
        ((rightExpander.module->model == modelMixMModule) || (rightExpander.module->model == modelMix4Module)) &&
        !amMaster();

    // A MixM and a Mix4 can both pair with a Mix4 to the left
    const bool pairedLeft = leftExpander.module &&
        (leftExpander.module->model == modelMix4Module);

    assert(rightProducerMessage);
    assert(!pairedLeft || leftModule->rightConsumerMessage);

    // set a channel to send data to the right (case #1, above)
    setExternalOutput(pairedRight ? reinterpret_cast<float *>(rightExpander.producerMessage) : nullptr);
    
    // set a channel to rx data from the left (case #2, above)
    setExternalInput(pairedLeft ? reinterpret_cast<float *>(leftExpander.module->rightExpander.consumerMessage) : nullptr);

    if (soloRequestFromUI != SoloCommands::DO_NOTHING) {
        const auto commCmd = (soloRequestFromUI == SoloCommands::SOLO_NONE) ?
            CommCommand_ClearAllSolo : CommCommand_ExternalSolo;

        // If solo requested, queue up solo commands for both sides    
        // TODO: these should only be done for exclusive solo 
        if (pairedRight) {
            sendRightChannel.send(commCmd);
        }
        if (pairedLeft) {
            sendLeftChannel.send(commCmd);
        }
      
        // tell our own module to solo, if a state change is requested
        if (soloRequestFromUI != currentSoloStatusFromUI) {
            requestModuleSolo(soloRequestFromUI);

            //and update our current state
            currentSoloStatusFromUI = soloRequestFromUI;
        }

        // Now that we have processed the command from UI, retire it.
        soloRequestFromUI = SoloCommands::DO_NOTHING;

    }

    if (pairedRight) {
        // #1) Send data to right: use you own right producer buffer.
        uint32_t* outBuf = reinterpret_cast<uint32_t *>(rightExpander.producerMessage);

        // #4) Receive data from right: user right's left consumer buffer
        const uint32_t* inBuf = reinterpret_cast<uint32_t *>(rightExpander.module->leftExpander.consumerMessage);
        sendRightChannel.go(outBuf + 4);
        uint32_t cmd = receiveRightChannel.rx(inBuf + 0);

        if (cmd != 0) {
            // iI the command is external solo, then we want to turn off all our
            // channles to let the other module have exclusive solo.
            // Otherwise the command is CommCommand_ClearAllSolo, and we should
            // lift all our external overrides.
            const SoloCommands reqMuteStatus = (cmd == CommCommand_ExternalSolo) ? 
                SoloCommands::SOLO_ALL :  SoloCommands::SOLO_NONE;
            //printf("right read status change (%d) module=%p \n", (int)reqMuteStatus, this); fflush(stdout);

            requestModuleSolo(reqMuteStatus);
            // now relay down to the left
            if (pairedLeft) {
                 sendLeftChannel.send(cmd);
            }
        }
    }

    if (pairedLeft) {
        // #3) Send data to the left: use your own left producer buffer.
        uint32_t* outBuf = reinterpret_cast<uint32_t *>(leftExpander.producerMessage);
        
        // #2) Receive data from left:  use left's right consumer buffer
        const uint32_t* inBuf = reinterpret_cast<uint32_t *>(leftExpander.module->rightExpander.consumerMessage);
        sendLeftChannel.go(outBuf + 0);
        uint32_t cmd = receiveLeftChannel.rx(inBuf + 4);
        if (cmd != 0) {
            const SoloCommands reqMuteStatus = (cmd == CommCommand_ExternalSolo) ? 
                SoloCommands::SOLO_ALL :  SoloCommands::SOLO_NONE;
            //printf("left read status change (%d) module=%p \n", (int)reqMuteStatus, this); fflush(stdout);

            requestModuleSolo(reqMuteStatus);
            // now relay down to the right
            if (pairedRight) {
                 sendRightChannel.send(cmd);
            }
        }
    }

    // Do the audio processing, and handle the left and right audio buses
    internalProcess();

    if (pairedRight) {
        rightExpander.messageFlipRequested = true;
    }
    if (pairedLeft) {
        leftExpander.messageFlipRequested = true;
     }
}


inline void MixerModule::requestSoloFromUI(SoloCommands command)
{
    //printf("\nUI req solo %d state =%d module=%p\n", (int) command, (int) currentSoloStatusFromUI, this); fflush(stdout);

    // Is it a request to turn off an already soloing channel,
    // thus clearing all solos?
    if (currentSoloStatusFromUI == command) {
        soloRequestFromUI = SoloCommands::SOLO_NONE;
        //printf("UI req interpreted as un-solo\n");
    } else {
        soloRequestFromUI = command;        // Queue up a request for the audio thread.
                                            // TODO: use atomic?
    }
}

/********************************************************
 * support function added here for convenience
 * 
 */

template<class Comp>
inline void unSoloAllChannels(MixerModule* mod)
{
    engine::Engine* eng = APP->engine;
    for (int i=0; i<4; ++i) {
        eng->setParam(mod, i + Comp::SOLO0_PARAM, 0);  
    }
}

template<class Comp>
inline void processSoloRequestForModule(MixerModule* mod, SoloCommands command)
{
    //printf("processSoloRequestForModule %d\n", (int)command); fflush(stdout);
    engine::Engine* eng = APP->engine;
    switch (command) {
       case SoloCommands::SOLO_0:
            //printf("processSoloRequestForModule SOLO_0\n"); fflush(stdout);
            unSoloAllChannels<Comp>(mod);
            eng->setParam(mod, Comp::SOLO0_PARAM, 1.f);      
            break;
        case SoloCommands::SOLO_1:
            //printf("processSoloRequestForModule SOLO_1\n"); fflush(stdout);
            unSoloAllChannels<Comp>(mod);
            eng->setParam(mod, Comp::SOLO1_PARAM, 1.f);      
            break;
        case SoloCommands::SOLO_2:
            //printf("processSoloRequestForModule SOLO_2\n"); fflush(stdout);
            unSoloAllChannels<Comp>(mod);
            eng->setParam(mod, Comp::SOLO2_PARAM, 1.f);      
            break;
        case SoloCommands::SOLO_3:
            //printf("processSoloRequestForModule SOLO_3\n"); fflush(stdout);
            unSoloAllChannels<Comp>(mod);
            eng->setParam(mod, Comp::SOLO3_PARAM, 1.f);      
            break;
        case SoloCommands::SOLO_NONE:
            unSoloAllChannels<Comp>(mod);
            break;
        case SoloCommands::SOLO_ALL:
            // we should turn off all channels, but clear all solo lights
            unSoloAllChannels<Comp>(mod);
            printf("we still need to (audi) mute our whole module now\n"); fflush(stdout);
            break;
        default:
        /*   SOLO_ALL,           // 8
    SOLO_NONE,          
    DO_NOTHING, */
            printf("processSoloRequestForModule %d, but nimp (all=8, none=9, nothing=10\n", (int)command); fflush(stdout);     
   }   
}
