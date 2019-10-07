
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
     * Concrete subclass implements this to accept a request
     * for solo change. It will be on the audio thread
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

    std::shared_ptr<SharedSoloState> getSharedSoloState() {
        return sharedSoloState;
    }
   
    int getModuleIndex()
    {
        return moduleIndex;
    }

    virtual int getNumGroups() const = 0;
    virtual int getMuteAllParam() const = 0;
    virtual int getSolo0Param() const = 0;

    void sendSoloChangedMessageOnAudioThread()
    {
        pleaseSendSoloChangedMessageOnAudioThread = true;
    }
protected:

    /**
     * Concrete subclass overrides these to transfer audio
     * with neighbors. subclass will fill input and consume output
     * on its process call.
     */
    virtual void setExternalInput(const float*)=0;
    virtual void setExternalOutput(float*)=0;

      // only master should call this
    void allocateSharedSoloState();

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

    // This guy holds onto a shared solo state, and we pass weak pointers to
    // him to clients so they can work.
    std::shared_ptr<SharedSoloStateOwner> sharedSoloStateOwner;

    // both masters and expanders hold onto these.
    std::shared_ptr<SharedSoloState> sharedSoloState;

    bool pleaseSendSoloChangedMessageOnAudioThread = false;

    int moduleIndex = 0;
    int pingDelayCount = 0;

    void pollAndProccessCommandFromUI(bool pairedRight, bool pairedLeft);
    void processMessageFromBus(const CommChannelMessage& msg, bool isEndOfMessageChain);
    void pollForModulePing(bool pairedLeft);
    void onSomethingChanged();

};

// Remember : "rightModule" is the module to your right
// producer and consumer are concepts of the engine, not us.
// rack will flip producer and consumer each process tick.
inline MixerModule::MixerModule()
{
    // rightExpander is a field from rack::Module.
    rightExpander.producerMessage = bufferFlipR;
    rightExpander.consumerMessage = bufferFlopR;
    leftExpander.producerMessage = bufferFlipL;
    leftExpander.consumerMessage = bufferFlopL;
}

inline void MixerModule::allocateSharedSoloState()
{
    assert(!sharedSoloStateOwner);
    sharedSoloStateOwner = std::make_shared<SharedSoloStateOwner>();
    sharedSoloState =  sharedSoloStateOwner->state; 
}

inline void MixerModule::pollForModulePing(bool pairedLeft)
{
#if 1
    if (amMaster()) {
        if (pingDelayCount-- <= 0) {
            pingDelayCount = 100;       // poll every 100 samples?
          //  pingDelayCount = 100000;        // for debugging, do less often
            assert(sharedSoloStateOwner);

            // now, if paired left, send a message to the right
            if (pairedLeft) {

                // TODO: move this allocation off the audio thread!!
                SharedSoloStateClient* stateForClient = new SharedSoloStateClient(sharedSoloStateOwner);
                assert(moduleIndex == 0);
                stateForClient->moduleNumber = 1;
                CommChannelMessage msg;
                msg.commandId = CommCommand_SetSharedState;
                msg.commandPayload = size_t(stateForClient);
                sendLeftChannel.send(msg);

               // WARN("setting shared solo from master %d", !!sharedSoloState);
            }
        }
    }
    #endif
}

inline void MixerModule::processMessageFromBus(const CommChannelMessage& msg, bool isEndOfMessageChain)
{
    // TODO: why are we getting this?
    if (msg.commandId == 0) {
        // WARN("blowing off ");
        return;
    }

    switch(msg.commandId) {
        case CommCommand_SetSharedState:
            {
                SharedSoloStateClient* stateForClient = reinterpret_cast<SharedSoloStateClient*>(msg.commandPayload);
                std::shared_ptr<SharedSoloStateOwner> owner = stateForClient->owner.lock();

                // If then owner has been deleted, then bail
                if (!owner) {
                    delete stateForClient;
                    sharedSoloState.reset();
                    return;
                }

                sharedSoloState = owner->state;
               // WARN("setting shared solo from message %d", !!sharedSoloState);
                moduleIndex = stateForClient->moduleNumber++;

                if (isEndOfMessageChain) {
                    delete stateForClient;
                }
            }
            break;
        case CommCommand_SomethingChanged:
            onSomethingChanged();
            break;
        default:
            WARN("no handler for message %x", msg.commandId);
    }
  //  WARN("processCommandFromBus does nothing now msg =%x payload=%p this = %p",
  //   msg.commandId, msg.commandPayload, this);
    #if 0   // old way
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
    #endif
}

inline void MixerModule::pollAndProccessCommandFromUI(bool pairedRight, bool pairedLeft)
{
    // If there is a request from the UI
    if (soloRequestFromUI != SoloCommands::DO_NOTHING) {
#if 0
        WARN("pollAndProccessCommandFromUI does nothing now");
#else
    //  const auto commCmd = (soloRequestFromUI == SoloCommands::SOLO_NONE) ?
    //      CommCommand_ClearAllSolo : CommCommand_ExternalSolo;
    CommChannelMessage msg;
    msg.commandId = CommCommand_SomethingChanged;
    WARN("why is this still called? this is the old way, yes?");

    // notify everyone that something changed/
    // TODO: who is going to update the shared status?
    if (pairedRight) {
        WARN("relay right");
        sendRightChannel.send(msg);
    }
    if (pairedLeft) {
        WARN("relay left %x", msg.commandId);
        sendLeftChannel.send(msg);
    }
    
    // tell our own module to solo, if a state change is requested
    // TODO: this won't support multi
    if (soloRequestFromUI != currentSoloStatusFromUI) {
        //printf("requesting module solo %d from 129\n", (int) soloRequestFromUI); fflush(stdout);
        requestModuleSolo(soloRequestFromUI);

        //and update our current state
        currentSoloStatusFromUI = soloRequestFromUI;
    }
    #endif

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

    pollForModulePing(pairedLeft);
    pollAndProccessCommandFromUI(pairedRight, pairedLeft);

    if (pairedRight) {
        // #1) Send data to right: use you own right producer buffer.
        uint32_t* outBuf = reinterpret_cast<uint32_t *>(rightExpander.producerMessage);

        // #4) Receive data from right: user right's left consumer buffer
        const uint32_t* inBuf = reinterpret_cast<uint32_t *>(rightExpander.module->leftExpander.consumerMessage);
        
        sendRightChannel.go(
            outBuf + comBufferRightCommandIdOffset,
            (size_t*)(outBuf + comBufferRightCommandDataOffset));

        CommChannelMessage msg;
        const bool isCommand = receiveRightChannel.rx(
            inBuf + comBufferLeftCommandIdOffset,
            (size_t*)(inBuf + comBufferLeftCommandDataOffset),
            msg);

        if (isCommand) {
            processMessageFromBus(msg, !pairedLeft);
            // now relay down to the left
            if (pairedLeft) {
                 sendLeftChannel.send(msg);
            }
        } // else WARN("got no command from right");
        if (pleaseSendSoloChangedMessageOnAudioThread) {
            CommChannelMessage msg2;
            msg2.commandId = CommCommand_SomethingChanged;
            sendRightChannel.send(msg2);
        }
    }

    if (pairedLeft) {
        // #3) Send data to the left: use your own left producer buffer.
        uint32_t* outBuf = reinterpret_cast<uint32_t *>(leftExpander.producerMessage);
        
        // #2) Receive data from left:  use left's right consumer buffer
        const uint32_t* inBuf = reinterpret_cast<uint32_t *>(leftExpander.module->rightExpander.consumerMessage);
        
        sendLeftChannel.go(
            outBuf + comBufferLeftCommandIdOffset,
            (size_t*)(outBuf + comBufferLeftCommandDataOffset));
        
        CommChannelMessage msg;
        const bool isCommand = receiveLeftChannel.rx(
            inBuf + comBufferRightCommandIdOffset,
            (size_t*)(inBuf + comBufferRightCommandDataOffset),
            msg);
        if (isCommand) {
            processMessageFromBus(msg, !pairedRight);
            if (pairedRight) {
                 sendRightChannel.send(msg);
            }
        }
        if (pleaseSendSoloChangedMessageOnAudioThread) {
            CommChannelMessage msg2;
            msg2.commandId = CommCommand_SomethingChanged;
            sendLeftChannel.send(msg2);
        }
    }
  
    pleaseSendSoloChangedMessageOnAudioThread = false;
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
    WARN("why is this called now 404");
    //printf("requestSoloFromUI %d\n", int(command));
    soloRequestFromUI = command;       // Queue up a request for the audio thread.                                  // TODO: use atomic?
}

inline void dumpState(const char* title,  std::shared_ptr<SharedSoloState> state) {
    DEBUG("    state: %s", title);
    DEBUG("        excl=%d,%d,%d", 
        !!state->state[0].exclusiveSolo,
        !!state->state[1].exclusiveSolo,
        !!state->state[2].exclusiveSolo);
    DEBUG("        multi=%d,%d,%d", 
        !!state->state[0].multiSolo,
        !!state->state[1].multiSolo,
        !!state->state[2].multiSolo);
}

// called from audio thread
inline void MixerModule::onSomethingChanged()
{
    DEBUG("on something changed");
    if (!sharedSoloState) {
        WARN("something changed, but no state");
        return;
    }

    if (moduleIndex >= SharedSoloState::maxModules) {
        WARN("too many modules");
        return;
    }

    dumpState("start of something changed ", sharedSoloState);

    bool otherModuleHasSolo = false;
    bool thisModuleHasSolo = false;
    bool otherModuleHasExclusiveSolo = false;
    for (int i=0; i<SharedSoloState::maxModules; ++i) {
        if (i != moduleIndex) {
            otherModuleHasSolo |= sharedSoloState->state[i].exclusiveSolo;
            otherModuleHasSolo |= sharedSoloState->state[i].multiSolo;
            otherModuleHasExclusiveSolo |= sharedSoloState->state[i].exclusiveSolo;
        } else {
            thisModuleHasSolo |= sharedSoloState->state[i].exclusiveSolo;
            thisModuleHasSolo |= sharedSoloState->state[i].multiSolo;
        }
     }
     // case : I have multi and other guy has exclusive

     DEBUG("    on-ch, thissolo = %d othersolo = %d otherexclusive = %d\n",
        thisModuleHasSolo, 
        otherModuleHasSolo,
        otherModuleHasExclusiveSolo);

    engine::Engine* eng = APP->engine;

    const bool thisModuleShouldMute = 
        (otherModuleHasSolo && !thisModuleHasSolo) ||
        otherModuleHasExclusiveSolo;
    DEBUG("    thisModuleShouldMute = %d", thisModuleShouldMute);
    eng->setParam(this, getMuteAllParam(), thisModuleShouldMute ? 1.f : 0.f); 

    if (otherModuleHasExclusiveSolo) {
        DEBUG("    on something changed - other module has exclusive so clear our solos");
        for (int i=0; i< this->getNumGroups(); ++i ) {
            const int paramNum =  this->getSolo0Param() + i;
            eng->setParam(this, paramNum, 0.f);
        }
    }

     dumpState("leave something changed ", sharedSoloState);
}

/********************************************************
 * Support function added here for convenience.
 * Put in their own namespace: sqmix.
 */

namespace sqmix {


/**
 * my current thinking is that we should update all the params for our module here,
 * and update the share solo state.
 * then we just need to ding the mixer module to send the changed message
 */
template<class Comp>
inline void handleSoloClickFromUI(MixerModule* mixer, int channel, bool ctrl)
{
   
    auto state = mixer->getSharedSoloState();
    int myIndex = mixer->getModuleIndex();
    if (!state) {
        WARN("can't get shared state");
        return;
    }
    if (myIndex >= SharedSoloState::maxModules) {
        WARN("too many modules");
        return;
    }
     DEBUG("** handleSoloClickFromUI, ctrl = %d myIndex = %d\n", ctrl, myIndex);
    // only worry about exclusive, for now
    const int channelParamNum =  Comp::SOLO0_PARAM + channel;

    // before processing this button - is the solo on?
    const bool groupIsSoloing = APP->engine->getParam(mixer, channelParamNum);
    const bool groupIsSoloingAfter = !groupIsSoloing;
    engine::Engine* eng = APP->engine;

    // if any of our groups are soloing, the module must be
    bool moduleIsSoloingAfter = groupIsSoloingAfter;
    for (int i=0; i< mixer->getNumGroups(); ++i ) {
        const int paramNum =  Comp::SOLO0_PARAM + i;
        if (i == channel) {
            // toggle the one we clicked on
            eng->setParam(mixer, paramNum, groupIsSoloing ? 0.f : 1.f);
        } else {
          
            if (!ctrl) {
                // if it's exclusive, turn off other channels
                eng->setParam(mixer, paramNum, 0.f);     
            } else {
                const bool soloing = APP->engine->getParam(mixer, paramNum);
                moduleIsSoloingAfter |= soloing;
            }
        }
    }

    // in both cases we need to un mute the module
    eng->setParam(mixer, Comp::ALL_CHANNELS_OFF_PARAM, 0); 

    // now update the shared state. If we are exclusive we must
    // clear others and set ours.
    // if we were exclusive and un-soloed, we must clear ourlf
   
    const bool isExclusive = !ctrl;

    DEBUG("    groupIsSoloingAfter = %d isExclusive = %d moduleSolo=%d", 
        groupIsSoloingAfter, 
        isExclusive,
        moduleIsSoloingAfter);

    dumpState("before update", state);
    for (int i=0; i<SharedSoloState::maxModules; ++i) {
        const bool isMe = (i == myIndex);
        if (isMe) {
            if (isExclusive) {
                // set our own exclusive state to match our solo.
                state->state[i].exclusiveSolo = groupIsSoloingAfter;
                state->state[i].multiSolo = false;
                DEBUG("set multi[%d] false because isExclusive", i);
            } else {
                state->state[i].exclusiveSolo = false;
                state->state[i].multiSolo = moduleIsSoloingAfter;
                 DEBUG("set multi[%d] to %d because moduleIsSoloingAfter flag", i, !!state->state[i].multiSolo);
            }
        }

        if (!isMe && isExclusive && groupIsSoloingAfter) {
            // if we just enabled and exclusive solo, clear everyone else.
            DEBUG("   clearing exclusive in module %d", i);
            state->state[i].exclusiveSolo = false;
        }
    }

    dumpState("after update", state);


    mixer->sendSoloChangedMessageOnAudioThread();
}

#if 0 // old version
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
    WARN("process exclusive solo");
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
     WARN("process multi solo");
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
            INFO("processSoloRequestForModule is shutting off");
            eng->setParam(mod, Comp::ALL_CHANNELS_OFF_PARAM, 1);
            break;
        default:
            printf("processSoloRequestForModule %d, but NIMP (all=8, none=9, nothing=10\n", (int)command); fflush(stdout);     
   }   
}
#endif

}   // end namespace
