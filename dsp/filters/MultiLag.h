#pragma once

#include "AudioMath.h"
#include "LowpassFilter.h"

#include <assert.h>
#include <cmath>
#include <xmmintrin.h>
#include <mmintrin.h>

#define _LLOOK
#define _LPSSE

/**
 * initial CPU = 3.0, 54.1 change freq every sample
 */
template <int N>
class MultiLag
{
public:
    /**
     * attack and release specified as normalized frequency (LPF equivalent)
     */
    void setAttack(float);
    void setRelease(float);
    void step(const float * buffer);
    float get(int index) const
    {
        assert(index < N);
        return memory[index];
    }
private:
    float memory[N] = {0};

    float lAttack = 0;
    float kAttack = 0;
    float lRelease = 0;
    float kRelease = 0;
};

template <int N>
inline void MultiLag<N>::setAttack(float fs)
{
    assert(fs > 00 && fs < .5);
    kAttack = float(1.0 - (std::exp(-2.0 * AudioMath::Pi * fs)));
    lAttack = float(1.0 - kAttack);
}

template <int N>
inline void MultiLag<N>::setRelease(float fs)
{
    assert(fs > 00 && fs < .5);
    kRelease = float(1.0 - (std::exp(-2.0 * AudioMath::Pi * fs)));
    lRelease = float(1.0 - kRelease);
}

/**
 * z = _z * _l + _k * x;
 */
template <int N>
inline void MultiLag<N>::step(const float * input)
{
    for (int i = 0; i < N; ++i) {
        if (input[i] > memory[i]) {
            memory[i] = memory[i] * lAttack + kAttack * input[i];
        } else {
            memory[i] = memory[i] * lRelease + kRelease * input[i];
        }
    }
}

/******************************************************************************************************/

/**
 * initial CPU = 2.3, 28.1 change freq every sample
 *                  , 6.1 with non-uniform look
 *      1.2 with SSE version
 */
template <int N>
class MultiLPF
{
public:
    // normalized cutoff freq
    void setCutoff(float);

    void step(const float * buffer);

    float get(int index) const
    {
        assert(index < N);
        return memory[index];
    }

private:
    float memory[N] = {0};


#ifdef _LPSSE
    __m128 l;
    __m128 k;

#else
    float l = 0;
    float k = 0;
#endif

#ifdef _LLOOK
    std::shared_ptr<NonUniformLookupTableParams<float>> lookup = makeLPFilterLookup<float>();
#endif
};


template <int N>
inline void MultiLPF<N>::setCutoff(float fs)
{
    assert(fs > 00 && fs < .5);
#if defined(_LLOOK) && !defined(_LPSSE)
    k = NonUniformLookupTable<float>::lookup(*lookup, fs);
    l = float(1.0 - k);
#elif !defined(_LLOOK) && !defined(_LPSSE) 
    k = float(1.0 - (std::exp(-2.0 * AudioMath::Pi * fs)));
    l = float(1.0 - k);
#elif  defined(_LLOOK) && defined(_LPSSE)
    float ks = NonUniformLookupTable<float>::lookup(*lookup, fs);
    float ls = float(1.0 - ks);
    k = _mm_set_ps1(ks);
    l = _mm_set_ps1(ls);
#else
    assert(false);
#endif
   
}


/**
 * z = _z * _l + _k * x;
 */
#if !defined(_LPSSE)
template <int N>
inline void MultiLPF<N>::step(const float * input)
{
    for (int i = 0; i < N; ++i) {
        memory[i] = memory[i] * l + k * input[i];
    }
}
#else
template <int N>
inline void MultiLPF<N>::step(const float * input)
{
    assert((N % 4) == 0);
    for (int i = 0; i < N; i += 4) {
        __m128 input4 = _mm_loadu_ps(input +i);  // load 4 input samples
        __m128 memory4 = _mm_loadu_ps(memory + i);

        __m128 temp = _mm_mul_ps(input4, k);
        memory4 = _mm_mul_ps(memory4, l);
        memory4 = _mm_add_ps(memory4, temp);
        _mm_storeu_ps(memory + i, memory4);
       

        //memory[i] = memory[i] * l + k * input[i];
    }
}
#endif
