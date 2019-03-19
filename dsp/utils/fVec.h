#pragma once

#include <cstdlib>
#include <xmmintrin.h>

template <int N>
class fVec
{
public:
    __m128 data[N];     // for now, let's not worry about alignment
    float* get();
    void set(const float * data);        // this = data
    void zero();

    /**
     * func_i is the "i place" version of func
     */
    void add_i(const fVec<N>& other);       // this += other
    void mul_i(const fVec<N>& other);       // this *= other
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
inline void fVec<N>::set(const float* extData)
{
    //return reinterpret_cast<float*> (&data[0]);
    for (int i = 0; i < N; ++i) {
        __m128 temp = _mm_loadu_ps(extData + (4 * i));
        data[i] = temp;
    }
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