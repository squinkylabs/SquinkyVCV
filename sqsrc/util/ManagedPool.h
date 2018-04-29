#pragma once


/**
 * A very specialized container. Made for holding one free 
 * work buffers, and making sure they are destroyed.
 *
 * At construction time, objects are created to fill the pool.
 * pop the objects to get one, push back to return when done.
 *
 * Destructor will delete all the objects, even if they are not in the pool
 * at the time.
 *
 * Note that unlike RingBuffer, ManagePool manages T*, not T
 */
template <typename T, int SIZE>
class ManagedPool
{
public:
    ManagedPool();


    /** Accessors from RingBuffer
     */
    void push(T*);
    T* pop();
    bool full() const;
    bool empty() const;
private:

    RingBuffer<T*, SIZE> ringBuffer;

};

template <typename T, int SIZE>
inline ManagedPool<T, SIZE>::ManagedPool()
{
    for (int i = 0; i < SIZE; ++i) {
        ringBuffer.push(new T());
    }
}

template <typename T, int SIZE>
inline void ManagedPool<T, SIZE>::push(T* value)
{
    ringBuffer.push(value);
}


template <typename T, int SIZE>
inline T* ManagedPool<T, SIZE>::pop()
{
    return ringBuffer.pop();
}

template <typename T, int SIZE>
inline bool ManagedPool<T, SIZE>::full() const
{
    return ringBuffer.full();
}

template <typename T, int SIZE>
inline bool ManagedPool<T, SIZE>::empty() const
{
    return ringBuffer.empty();
}


