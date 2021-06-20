#pragma once

#include <cstdint>

#include "RingBuffer.h"
#include "SimdBlocks.h"
#include "SqLog.h"

/**
 *
 * Very specialized container for delaying 16 gates.
 * All I/O is via float_4, alghouth internally it uses int16
 */
template <int SIZE>
class GateDelay {
public:
    void addGates(const float_4&);
    void commit();
    float_4 getGates();

private:
    SqRingBuffer<uint16_t, SIZE + 1> ringBuffer;
    int gatesAddedToFrame = 0;
    int gatesPulledFromFrame = 0;

    uint16_t addBuffer = 0;
    uint16_t getBuffer = 0;
};

template <int SIZE>
void GateDelay<SIZE>::addGates(const float_4& fourGates) {
    assert(gatesAddedToFrame < 4);
    auto x = rack::simd::movemask(fourGates);
    addBuffer |= (x << (gatesAddedToFrame * 4));
    ++gatesAddedToFrame;
    SQINFO("after add, num=%d val=%x", gatesAddedToFrame, addBuffer);
}

template <int SIZE>
void GateDelay<SIZE>::commit() {
    if (gatesAddedToFrame != 4) SQWARN("GateDelay not full");
    if (gatesAddedToFrame != 4) SQWARN("GateDelay not all read");

    ringBuffer.push(addBuffer);

    gatesAddedToFrame = 0;
    gatesPulledFromFrame = 0;
    addBuffer = 0;
    getBuffer = 0;
}

template <int SIZE>
float_4 GateDelay<SIZE>::getGates() {
    return float_4(0);
}