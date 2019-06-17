#pragma once

#include "RingBuffer.h"

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
 *  buffer[6] = commands
 * 
 * going right to left, only commands are sent:
 *  buffer[0] = command
 * 
 */

const int comBufferSizeRight = 7;
const int comBufferRightCommandOffset = 6;
const int comBufferSizeLeft = 1;
const int comBufferLeftCommandOffset = 0;

// This command sent when un-soloing. Receiver should clear it's solo status.
const uint32_t CommCommand_ClearAllSolo = (100 << 16); 

// This command sent when soloing. 
// Receiver should turn off all channels, as a different module will be soloing.
const uint32_t CommCommand_ExternalSolo = (101 << 16); 

/**
 * commands used by mixers for communicating sol info
 * (These are not sent over a comm channel, and don't really belong here)
 */

enum class SoloCommands {
    // SOLO_x normal, exclusive solo requested
    SOLO_0,
    SOLO_1,
    SOLO_2,
    SOLO_3,

    // SOLO_x_MULTI, non-exclusive "multi-solo"
    SOLO_0_MULTI,
    SOLO_1_MULTI,
    SOLO_2_MULTI,
    SOLO_3_MULTI,
    
    /**
     * (8)
     * mute all of your channels, because another module is 
     * requesting an exclusive solo.
     */
    SOLO_ALL,

    /**
     * (9)
     * remove the solo overrides from all your channels,
     * because another module stopped soloing
     */  
    SOLO_NONE,          
    DO_NOTHING,
};

/**
 * CommChannelSend
 * Sends messages from on VCV Module to another
 */
class CommChannelSend
{
public:
    const static int zeroPad = 3;       // number of zeros to send out after a command
    /**
     * Queues up a message to be sent.
     * Will be sent over several periods.
     */
    void send(uint32_t);

    /**
     * Must be called every sample period to run send 
     * state machine. 
     */
    void go(uint32_t * outputBuffer);
private:
    SqRingBuffer<uint32_t, 4> messageBuffer;
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
     * returns zero if no data received, otherwise returns data
     */
    uint32_t rx(const uint32_t * inputBuffer);
private:
    uint32_t lastCommand = 0;
};

inline void CommChannelSend::send(uint32_t msg)
{
    assert(!messageBuffer.full());
    messageBuffer.push(msg);
}

inline void CommChannelSend::go(uint32_t* output)
{
    uint32_t x=0;
    if (sendingZero) {
        if (++zeroCount >= zeroPad) {
            sendingZero = false;
        }
    } else if (sendingData) {
        sendingData = false;
        sendingZero = true;
        zeroCount = 1;

        x = 0;
    } else {
        if (!messageBuffer.empty()) {
            sendingData = true;
            x = messageBuffer.pop();
        }
    }
    //if (x != 0) printf("output 32: %x\n", x);
    *output = x;
}

inline uint32_t CommChannelReceive::rx(const uint32_t * inputBuffer)
{
    uint32_t ret = inputBuffer[0];
    if (ret == lastCommand) {
        ret = 0;
    } else {
        lastCommand = ret;
    }
    return ret;
};