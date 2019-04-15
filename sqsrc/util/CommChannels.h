#pragma once

#include "RingBuffer.h"

/**
 * CommChannelSend
 * Sends messages from on VCV Module to another
 */
class CommChannelSend
{
public:
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
};


/**
 * CommChannelReceive
 * Sends messages from on VCV Module to another
 */
class CommChannelReceive
{

};


void CommChannelSend::send(uint32_t msg)
{
    assert(!messageBuffer.full());
    messageBuffer.push(msg);
}


void CommChannelSend::go(uint32_t* output)
{
    uint32_t x=0;
    if (sendingZero) {
      
        sendingZero = false;
    } else if (sendingData) {
        sendingData = false;
        x = 0;
    } else {
        if (!messageBuffer.empty()) {
            sendingData = true;
            x = messageBuffer.pop();
        }
    }
    *output = x;
}

#if 0
void CommChannelSend::go(uint32_t* output)
{
    uint32_t x;
    if (sendingMsg) {
        x = messageBuffer.pop();
        sendingMsg = false;
    } else {
        x = 0;
    }
    *output = x;
}
#endif