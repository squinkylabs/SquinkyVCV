#pragma once

#include <cstdint>

#include "RingBuffer.h"
#include "SimdBlocks.h"

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
};

template <int SIZE>
void GateDelay<SIZE>::addGates(const float_4&) {
}

template <int SIZE>
void GateDelay<SIZE>::commit() {
}

template <int SIZE>
float_4 GateDelay<SIZE>::getGates() {
    return float_4(0);
}