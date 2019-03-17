#pragma once

#include "AudioMath.h"
#include "LookupTable.h"
#include "LowpassFilter.h"

#include <assert.h>
#include <cmath>
#include <xmmintrin.h>
#include <mmintrin.h>

#define _LLOOK
#define _LPSSE


/**
 * Taking original MultiLag, let's vectorize it better
 */
template <int N>
class MultiLag2
{
public:
    MultiLag2()
    {
        const float onef = 1.f;
        kOne = _mm_load1_ps(&onef);
    }
    /**
     * attack and release specified as normalized frequency (LPF equivalent)
     */
    void setAttack(float);
    void setRelease(float);

    /**
     * attack and release, using direct filter L values
     */
    void setAttackL(float l)
    {
        lAttack = _mm_set_ps1(l);
    }
    void setReleaseL(float l)
    {
        lRelease = _mm_set_ps1(l);
    }
    void setEnable(bool b)
    {
        enabled = b;
    }

    void step(const float * buffer);
    float get(int index) const
    {
        assert(index < N);
        return memory[index];
    }

private:
    float memory[N] = {0};

    __m128 lAttack = {0};
    __m128 lRelease = {0};
    __m128 kOne = {1};
    bool enabled = true;

    std::shared_ptr<NonUniformLookupTableParams<float>> lookup = makeLPFilterL_Lookup<float>();
};


#if defined(_LLOOK) && defined(_LPSSE)
template <int N>
inline void MultiLag2<N>::setAttack(float fs)
{
    assert(fs > 00 && fs < .5);
    float ls = NonUniformLookupTable<float>::lookup(*lookup, fs);
    lAttack = _mm_set_ps1(ls);
}


template <int N>
inline void MultiLag2<N>::setRelease(float fs)
{
    assert(fs > 00 && fs < .5);
    float ls = NonUniformLookupTable<float>::lookup(*lookup, fs);
    lRelease = _mm_set_ps1(ls);
}

/**
 * z = _z * _l + _k * x;
 */
template <int N>
inline void MultiLag2<N>::step(const float * input)
{
    if (!enabled) {
        for (int i = 0; i < N; i += 4) {
            __m128 input4 = _mm_loadu_ps(input + i);  // load 4 input samples
            _mm_storeu_ps(memory + i, input4);
        }
        return;
    }
    assert((N % 4) == 0);
    for (int i = 0; i < N; i += 4) {
        __m128 input4 = _mm_loadu_ps(input + i);  // load 4 input samples
        __m128 memory4 = _mm_loadu_ps(memory + i);
        __m128 cmp = _mm_cmpge_ps(input4, memory4);     //cmp has 11111 where >=, 0000 others

#if 0 // old way, with k
        __m128 ka = _mm_and_ps(cmp, kAttack);
        __m128 kr = _mm_andnot_ps(cmp, kRelease);
        __m128 k = _mm_or_ps(ka, kr);

        __m128 l = _mm_sub_ps(kOne, k);
#else
        __m128 la = _mm_and_ps(cmp, lAttack);
        __m128 lr = _mm_andnot_ps(cmp, lRelease);
        __m128 l = _mm_or_ps(la, lr);
        __m128 k = _mm_sub_ps(kOne, l);
#endif

        // now k and l have the correct a/r time constants

        __m128 temp = _mm_mul_ps(input4, k);
        memory4 = _mm_mul_ps(memory4, l);
        memory4 = _mm_add_ps(memory4, temp);
        _mm_storeu_ps(memory + i, memory4);
    }
}
#endif
