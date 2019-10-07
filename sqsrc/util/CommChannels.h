#pragma once

#include "RingBuffer.h"

#include <cstdint>
#include <atomic>
#include <memory>

/**
 * Command Protocol:
 *      We send one unit32_t, followed by 'n' zeros
 *      zero is not a legal command
 * 
 *      Top 16 bits are the command, bottom 16 are the data
 * 
 * Data protocol: VCV provides are array of 32-bit floats that you may use for message passing.
 * 
 * going left to right we send a lot of busses:
 *  buffer[0] = left master buss
 *  buffer[1] = right master buss
 *  buffer[2] = left aux A buss
 *  buffer[3] = right aux A buss
 *  buffer[4] = left aux B bus
 *  buffer[5] = right aux B bus
 *  buffer[6] = commands id
 *  buffer[7, 8] = commands payload
 * 
 * going right to left, only commands are sent:
 *  buffer[0] = command id
 *  buffer[1,2] = commands payload
 * 
 */

const int comBufferSizeRight = 9;
const int comBufferRightCommandIdOffset = 6;
const int comBufferRightCommandDataOffset = 7;

const int comBufferSizeLeft = 3;
const int comBufferLeftCommandIdOffset = 0;
const int comBufferLeftCommandDataOffset = 1;

// This command sent when un-soloing. Receiver should clear it's solo status.
//const uint32_t CommCommand_ClearAllSolo = (100 << 16); 

// This command sent when soloing. 
// Receiver should turn off all channels, as a different module will be soloing.
//const uint32_t CommCommand_ExternalSolo = (101 << 16); 

const uint32_t CommCommand_SetSharedState = (102 << 16); 
const uint32_t CommCommand_SomethingChanged = (103 << 16); 

class CommChannelMessage
{
public:
    uint32_t    commandId = 0;
    size_t      commandPayload = 0;
};

/**
 * CommChannelSend
 * Sends messages from on VCV Module to another
 */
class CommChannelSend
{
public:
    const static int zeroPad = 3;       // number of zeros to send out after a command
                                        // (I don't know why we do this now, but we do)
    /**
     * Queues up a message to be sent.
     * Will be sent over several periods.
     */
    void send(const CommChannelMessage& message);

    /**
     * Must be called every sample period to run send 
     * state machine. 
     */
    void go(uint32_t* outputCommandBuffer, size_t* outputDataBuffer);
    CommChannelSend() : messageBuffer(false) {}
private:
    SqRingBuffer<CommChannelMessage, 4> messageBuffer;
    bool sendingData = false;
    bool sendingZero = false;
    int zeroCount = 0;
};

/**
 * CommChannelReceive
 * Sends messages from on VCV Module to another
 */
class CommChannelReceive
{
public:
    /**
     * returns false if no data received, otherwise returns data in msg
     */
    bool rx(const uint32_t * inputCommandBuffer, const size_t* inputDataBuffer, CommChannelMessage& msg);
private:
    uint32_t lastCommand = 0;
};

inline void CommChannelSend::send(const CommChannelMessage& msg)
{
    assert(!messageBuffer.full());
    messageBuffer.push(msg);
}

inline void CommChannelSend::go(uint32_t* outputCommandBuffer, size_t* outputDataBuffer)
{
   // uint32_t x=0;
    CommChannelMessage message;
    if (sendingZero) {
        if (++zeroCount >= zeroPad) {
            sendingZero = false;
        }
    } else if (sendingData) {
        sendingData = false;
        sendingZero = true;
        zeroCount = 1;
    } else {
        if (!messageBuffer.empty()) {
            sendingData = true;
            message = messageBuffer.pop();
        }
    }

   *outputCommandBuffer = message.commandId;
   *outputDataBuffer = message.commandPayload;
}

inline bool CommChannelReceive::rx(const uint32_t * inputCommandBuffer, const size_t* inputDataBuffer, CommChannelMessage& msg)
{
    bool didReceive = false;
    const uint32_t newCommandId = *inputCommandBuffer;
    if (newCommandId != lastCommand) {
        msg.commandId = newCommandId;
        lastCommand = newCommandId;
        msg.commandPayload = *inputDataBuffer;
        didReceive = true;
    }
    return didReceive;
};

/**
 * There is one of these, but all the modules have access to it.
 * They use it to coordinate solo / multi-solo across modules.
 */
extern int soloStateCount;      // just for debugging
class SharedSoloState
{
public:
    SharedSoloState()
    {
        ++soloStateCount;
    }
    ~SharedSoloState()
    {
        --soloStateCount;
        //fprintf(stderr, "in dtor, solo state count = %d", soloStateCount);
    }
    static const int maxModules = 16;
    class State
    {
    public:
        std::atomic<bool> exclusiveSolo = {false};
        std::atomic<bool> multiSolo = {false};
    };

    State state[maxModules];
};

class SharedSoloStateOwner
{
public:
    SharedSoloStateOwner() {
        state = std::make_shared<SharedSoloState>();
    }
    std::shared_ptr<SharedSoloState> state;
};
 
class SharedSoloStateClient
{
public:
    SharedSoloStateClient(std::shared_ptr<SharedSoloStateOwner> own) {
        owner = std::weak_ptr<SharedSoloStateOwner>(own);
    }
    std::weak_ptr<SharedSoloStateOwner> owner;

    int moduleNumber = 0;
};
