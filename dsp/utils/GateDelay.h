#pragma once

#include <cstdint>

#include "RingBuffer.h"
#include "SimdBlocks.h"
#include "SqLog.h"

/**
 *
 * Very specialized container for delaying 16 gates.
 * All I/O is via float_4, although internally it uses int16
 */
template <int SIZE>
class GateDelay {
public:
    GateDelay();
    GateDelay(const GateDelay&) = delete;
    const GateDelay& operator = (const GateDelay&) = delete;
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


// Turn the ring buffer into a delay line by pushing enough zeros into it
template <int SIZE>
GateDelay<SIZE>::GateDelay() {
    // SQINFO("ctor of gate delay here is buffer");
    // ringBuffer._dump();
    for (int i=0; i<SIZE; ++i) {
        ringBuffer.push(0);
        // SQINFO("ctor of gate dela pushed one here is buffer");
        //ringBuffer._dump();
    }
}

template <int SIZE>
void GateDelay<SIZE>::addGates(const float_4& fourGates) {
    // SQINFO("enter add gate");
    assert(gatesAddedToFrame < 4);
    auto x = rack::simd::movemask(fourGates);
    addBuffer |= (x << (gatesAddedToFrame * 4));
    ++gatesAddedToFrame;
    // SQINFO("after add, num=%d val=%x", gatesAddedToFrame, addBuffer);
}

template <int SIZE>
void GateDelay<SIZE>::commit() {
    // SQINFO("enter commit, here is buffer: ");
    //ringBuffer._dump();
    if (gatesAddedToFrame != 4) SQWARN("GateDelay not full");
    if (gatesAddedToFrame != 4) SQWARN("GateDelay not all read");

    ringBuffer.push(addBuffer);

    gatesAddedToFrame = 0;
    gatesPulledFromFrame = 0;
    addBuffer = 0;
    getBuffer = ringBuffer.pop();
    // SQINFO("gate commit pushed into ring, popped %x", getBuffer);
    // SQINFO("Leaving commit\n");
    //ringBuffer._dump();
}

inline __m128 movemask_inverse_alternative(int x) {
     __m128i msk8421 = _mm_set_epi32(8, 4, 2, 1);
    __m128i x_bc = _mm_set1_epi32(x);
    __m128i t = _mm_and_si128(x_bc, msk8421);
    return _mm_castsi128_ps(_mm_cmpeq_epi32(msk8421, t));
}

template <int SIZE>
float_4 GateDelay<SIZE>::getGates() {
    // SQINFO("in get gate, pulled=%d", gatesPulledFromFrame);
    assert(gatesPulledFromFrame < 4);

    float_4 ret = 0;
    int retI = getBuffer >> (gatesPulledFromFrame * 4);
    retI &= 0xf;
    // SQINFO("in getGate int = %d", retI);
    auto temp = movemask_inverse_alternative(retI);
    ret = temp;
    gatesPulledFromFrame++;
   //  SQINFO("get returning %s", toStr(ret).c_str());
    return ret;
}
     
