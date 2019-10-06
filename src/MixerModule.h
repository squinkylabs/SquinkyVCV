
#pragma once

#include "CommChannels.h"
/**
 * This class manages the communication between
 * misers and mixer expanders.
 * 
 * How does threading work?
 * We receive solo requests from the UI on the UI thread, but queue those up
 * to handle on the audio thread.
 * 
 * All solo handling goes on on the audio thread, whether if came from our own UI
 * or from a different module over the expansion bus.
 * 
 * Modules just send SOLO commands to toggle themselve, and we re-interpret them
 * as requests to solo or un-solo. But can we do that with multi solos?
 * 
 * And should be be doing this on the audio thread at all? 
 * We can just as easily do it from the UI.
 * 
 * 
 * What happens for various user actions?
 * 
 * exclusive solo on a non-solo channel:
 *      solo self
 *      un-solo everyone else in self
 *      turn off all other modules.
 * 
 * exclusive solo on a solo channel:
 *      un-solo all channels in self
 *      turn on all modules
 * 
 * multi-solo on a non-solo channel
 *      solo self
 *      mute other modules unless they have a soloed channel
 * 
 * multi-colo on a soloed channel
 *      un-solo current channel
 *      perhaps turn off self (if other module is soloing)
 *      
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
    float bufferFlipR[comBufferSizeRight] = {0};
    float bufferFlopR[comBufferSizeRight] = {0};
    float bufferFlipL[comBufferSizeLeft] = {0};
    float bufferFlopL[comBufferSizeLeft] = {0};

    CommChannelSend sendRightChannel;
    CommChannelSend sendLeftChannel;
    CommChannelReceive receiveRightChannel;
    CommChannelReceive receiveLeftChannel;

    // This gets set when UI calls us ask us to do something
    SoloCommands soloRequestFromUI = SoloCommands::DO_NOTHING;

    // This is set by us after executing a request from UI.
    // We use it to track our state
    SoloCommands currentSoloStatusFromUI = SoloCommands::DO_NOTHING;

    void pollAndProccessCommandFromUI(bool pairedRight, bool pairedLeft);
    void processCommandFromBus(uint32_t cmd);
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

inline void MixerModule::processCommandFromBus(uint32_t cmd)
{
    // iI the command is external solo, then we want to turn off all our
    // channles to let the other module have exclusive solo.
    // Otherwise the command is CommCommand_ClearAllSolo, and we should
    // lift all our external overrides.
    const SoloCommands reqMuteStatus = (cmd == CommCommand_ExternalSolo) ? 
                SoloCommands::SOLO_ALL :  SoloCommands::SOLO_NONE;
            
    //printf("right read status change (%d) module=%p \n", (int)reqMuteStatus, this); fflush(stdout);
    
    // clear the UI status, so that UI can again solo in the future
    currentSoloStatusFromUI = SoloCommands::DO_NOTHING;
    requestModuleSolo(reqMuteStatus);
}

inline void MixerModule::pollAndProccessCommandFromUI(bool pairedRight, bool pairedLeft)
{
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
            //printf("requesting module solo %d from 129\n", (int) soloRequestFromUI); fflush(stdout);
            requestModuleSolo(soloRequestFromUI);

            //and update our current state
            currentSoloStatusFromUI = soloRequestFromUI;
        }

        // Now that we have processed the command from UI, retire it.
        soloRequestFromUI = SoloCommands::DO_NOTHING;

    }
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
        (   (rightExpander.module->model == modelMixMModule) ||
            (rightExpander.module->model == modelMix4Module) ||
            (rightExpander.module->model == modelMixStereoModule)
        ) &&
        !amMaster();

    // A MixM and a Mix4 can both pair with a Mix4 to the left
    const bool pairedLeft = leftExpander.module &&
        ((leftExpander.module->model == modelMix4Module) ||
        (leftExpander.module->model == modelMixStereoModule));

    // recently ported these asserts. Hope they are right.
    assert(rightExpander.producerMessage);
    assert(!pairedLeft || leftExpander.module->rightExpander.consumerMessage);

    // set a channel to send data to the right (case #1, above)
    setExternalOutput(pairedRight ? reinterpret_cast<float *>(rightExpander.producerMessage) : nullptr);
    
    // set a channel to rx data from the left (case #2, above)
    setExternalInput(pairedLeft ? reinterpret_cast<float *>(leftExpander.module->rightExpander.consumerMessage) : nullptr);

    pollAndProccessCommandFromUI(pairedRight, pairedLeft);

    if (pairedRight) {
        // #1) Send data to right: use you own right producer buffer.
        uint32_t* outBuf = reinterpret_cast<uint32_t *>(rightExpander.producerMessage);

        // #4) Receive data from right: user right's left consumer buffer
        const uint32_t* inBuf = reinterpret_cast<uint32_t *>(rightExpander.module->leftExpander.consumerMessage);
        sendRightChannel.go(outBuf + comBufferRightCommandOffset);
        uint32_t cmd = receiveRightChannel.rx(inBuf + comBufferLeftCommandOffset);

        if (cmd != 0) {
            processCommandFromBus(cmd);
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
        sendLeftChannel.go(outBuf + comBufferLeftCommandOffset);
        uint32_t cmd = receiveLeftChannel.rx(inBuf + comBufferRightCommandOffset);
        if (cmd != 0) {
             processCommandFromBus(cmd);
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



/**
 * Modules call this directly from their UI event handles.
 * This is where soling command processing starts
 */
inline void MixerModule::requestSoloFromUI(SoloCommands command)
{
    //printf("requestSoloFromUI %d\n", int(command));
    soloRequestFromUI = command;       // Queue up a request for the audio thread.                                  // TODO: use atomic?
}

/********************************************************
 * Support function added here for convenience.
 * Put in their own namespace: sqmix.
 */

namespace sqmix {

template<class Comp>
inline void handleSoloClickFromUI(MixerModule* mixer, int channel)
{
    
    const int paramNum =  Comp::SOLO0_PARAM + channel;
    const bool isSoloing = APP->engine->getParam(mixer, paramNum);
    //printf("handleSoloClickFromUI(%d) isSoling = %d\n", channel, isSoloing);
    SoloCommands cmd = isSoloing ? 
        SoloCommands::SOLO_NONE :
        SoloCommands(int(SoloCommands::SOLO_0) + channel);
    mixer->requestSoloFromUI(cmd); 
}

/**
 * clears all the SOLO params from the composites.
 */
template<class Comp>
inline void unSoloAllChannels(MixerModule* mod)
{
    engine::Engine* eng = APP->engine;
    for (int i=0; i < Comp::numGroups; ++i) {
        eng->setParam(mod, i + Comp::SOLO0_PARAM, 0);  
    }
}

template<class Comp>
inline void processExclusiveSolo(MixerModule* mod, SoloCommands command)
{
    engine::Engine* const eng = APP->engine;
    const int channel = int(command)- int(SoloCommands::SOLO_0);
    assert(channel >= 0 && channel < Comp::numGroups);
    unSoloAllChannels<Comp>(mod);
    //printf("processExclusiveSolo channel %d, will un-mute module\n", channel); fflush(stdout);
    eng->setParam(mod, Comp::SOLO0_PARAM + channel, 1.f); 
    eng->setParam(mod, Comp::ALL_CHANNELS_OFF_PARAM, 0);    
}

template<class Comp>
inline void processMultiSolo(MixerModule* mod, SoloCommands command)
{
    engine::Engine* const eng = APP->engine;
    const int channel = int(command)- int(SoloCommands::SOLO_0_MULTI);
    assert(channel >= 0 && channel < Comp::numGroups);

    const bool channelIsSoloed = eng->getParam(mod, Comp::SOLO0_PARAM + channel);

    //printf("nprocessMultiSolo(%d), is soloed = %d\n", channel, channelIsSoloed); fflush(stdout);

    // toggle the solo state of this one 
    eng->setParam(mod, Comp::SOLO0_PARAM + channel, channelIsSoloed ? 0 : 1); 
    eng->setParam(mod, Comp::ALL_CHANNELS_OFF_PARAM, 0);
}

/**
 * Called from modules as they process calls into their 
 * requestModuleSolo(SoloCommands command) functions.
 * This will be at the end of the call chain.
 */
template<class Comp>
inline void processSoloRequestForModule(MixerModule* mod, SoloCommands command)
{
    //printf("processSoloRequestForModule %d mod=%p\n", (int)command, mod); 
    //printf("SOLO_NONE = %d, SOLO_ALL = %d\n", (int)SoloCommands::SOLO_NONE, (int)SoloCommands::SOLO_ALL);
    //fflush(stdout);

    engine::Engine* eng = APP->engine;
    switch (command) {
        case SoloCommands::SOLO_0:
        case SoloCommands::SOLO_1:
        case SoloCommands::SOLO_2:
        case SoloCommands::SOLO_3:
            processExclusiveSolo<Comp>(mod, command);   
            break;
        case SoloCommands::SOLO_0_MULTI:
        case SoloCommands::SOLO_1_MULTI:
        case SoloCommands::SOLO_2_MULTI:
        case SoloCommands::SOLO_3_MULTI:
            processMultiSolo<Comp>(mod, command);   
            break;
        case SoloCommands::SOLO_NONE:
            //printf("got SOLO_NONE command\n"); fflush(stdout);
            unSoloAllChannels<Comp>(mod);
            eng->setParam(mod, Comp::ALL_CHANNELS_OFF_PARAM, 0);
            break;
        case SoloCommands::SOLO_ALL:
            //printf("got SOLO_ALL command\n"); fflush(stdout);
            // we should turn off all channels, and clear all solo lights
            unSoloAllChannels<Comp>(mod);
            eng->setParam(mod, Comp::ALL_CHANNELS_OFF_PARAM, 1);
            break;
        default:
            printf("processSoloRequestForModule %d, but NIMP (all=8, none=9, nothing=10\n", (int)command); fflush(stdout);     
   }   
}

}   // end namespace
