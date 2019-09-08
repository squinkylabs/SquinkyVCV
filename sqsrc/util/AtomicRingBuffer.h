#pragma once


#include <assert.h>
#include <atomic>

/**
 * A simple ring buffer.
 * Template arguments are for type stored, and for size.
 * Thread safe for single producer, single consumer case
 * Guaranteed to be non-blocking. Adding or removing items will never
 * allocate or free memory.
 * Objects in RingBuffer are not owned by RingBuffer - they will not be destroyed.
 */
template <typename T, int SIZE>
class AtomicRingBuffer
{
public:
    AtomicRingBuffer();
    void push(T);
    T pop();
    bool full() const;
    bool empty() const;
private:
    T memory[SIZE];
       
    std::atomic<int> size;  // must change this last, for consistency between threads.
    int inIndex = 0;
    int outIndex = 0;

     /** Move up 'p' (a buffer index), wrap around if we hit the end
      * (this is the core of the circular ring buffer).
      */
    void advance(int &p);
};

template <typename T, int SIZE>
inline AtomicRingBuffer<T, SIZE>::AtomicRingBuffer()
{
    size = 0;
}

template <typename T, int SIZE>
inline void AtomicRingBuffer<T, SIZE>::push(T value)
{
   assert(!full());
    memory[inIndex] = value;
    advance(inIndex);
    ++size;
}

template <typename T, int SIZE>
inline T AtomicRingBuffer<T, SIZE>::pop()
{
    assert(!empty());
    T value = memory[outIndex];
    advance(outIndex);
    --size;
    return value;
}

template <typename T, int SIZE>
inline bool AtomicRingBuffer<T, SIZE>::full() const
{
    return size == SIZE;
}

template <typename T, int SIZE>
inline bool AtomicRingBuffer<T, SIZE>::empty() const
{
    return size == 0;
}


template <typename T, int SIZE>
inline void AtomicRingBuffer<T, SIZE>::advance(int &p)
{
    if (++p >= SIZE) p = 0;
}




