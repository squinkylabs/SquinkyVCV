#pragma once

#include <cstdlib>
#include <xmmintrin.h>

template <int N>
class fVec
{
public:
    __m128 data[N];     // for now, let's not worry about alignment
    float* get();

    void zero();
    void add_i(const fVec<N>& other);
    void mul_i(const fVec<N>& other);
};

template <int N>
inline void fVec<N>::zero()
{
    for (int i = 0; i < N; ++i) {
        data[i] = _mm_set_ps1(0);
    }
}

template <int N>
inline float* fVec<N>::get()
{
    return reinterpret_cast<float*> (&data[0]);
}

template <int N>
inline void fVec<N>::add_i(const fVec<N>& other)
{
    for (int i = 0; i < N; ++i) {
        data[i] = _mm_add_ps(data[i], other.data[i]);
    }
}

template <int N>
inline void fVec<N>::mul_i(const fVec<N>& other)
{
    for (int i = 0; i < N; ++i) {
        data[i] = _mm_mul_ps(data[i], other.data[i]);
    }
}