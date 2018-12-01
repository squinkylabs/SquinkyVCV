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
 * initial CPU = 3.0, 54.1 change freq every sample
 * 2.1, 8.6 with lookup and SSE
 */
template <int N>
class MultiLag
{
public:
    MultiLag()
    {
        const float onef = 1.f;
        kOne = _mm_load1_ps(&onef);
    }
    /**
     * attack and release specified as normalized frequency (LPF equivalent)
     */
    void setAttack(float);
    void setRelease(float);
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

#ifdef _LPSSE
    __m128 kAttack = {0};
    __m128 kRelease = {0};
    __m128 kOne = {1};
#else
    float lAttack = 0;
    float kAttack = 0;
    float lRelease = 0;
    float kRelease = 0;
#endif
    bool enabled = true;

#ifdef _LLOOK
    std::shared_ptr<NonUniformLookupTableParams<float>> lookup = makeLPFilterLookup<float>();
#endif
};


#if defined(_LLOOK) && defined(_LPSSE)
template <int N>
inline void MultiLag<N>::setAttack(float fs)
{
    assert(fs > 00 && fs < .5);
    float ks = NonUniformLookupTable<float>::lookup(*lookup, fs);
  //  printf("ks attack set to %f\n", ks);
    kAttack = _mm_set_ps1(ks);
}


template <int N>
inline void MultiLag<N>::setRelease(float fs)
{
    assert(fs > 00 && fs < .5);
    float ks = NonUniformLookupTable<float>::lookup(*lookup, fs);
  //  printf("ks release set to %f\n", ks); fflush(stdout);
    kRelease = _mm_set_ps1(ks);
}

/**
 * z = _z * _l + _k * x;
 */
template <int N>
inline void MultiLag<N>::step(const float * input)
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

        __m128 ka = _mm_and_ps(cmp, kAttack);
        __m128 kr = _mm_andnot_ps(cmp, kRelease);
        __m128 k = _mm_or_ps(ka, kr);

        __m128 l = _mm_sub_ps(kOne, k);

        // now k and l have the correct a/r time constants

        __m128 temp = _mm_mul_ps(input4, k);
        memory4 = _mm_mul_ps(memory4, l);
        memory4 = _mm_add_ps(memory4, temp);
        _mm_storeu_ps(memory + i, memory4);
    }
}
#endif


//*************************************************************************
#if !defined(_LLOOK) && !defined(_LPSSE)
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
#endif

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
        __m128 l = {0};
        __m128 k = {0};

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
            __m128 input4 = _mm_loadu_ps(input + i);  // load 4 input samples
            __m128 memory4 = _mm_loadu_ps(memory + i);

            __m128 temp = _mm_mul_ps(input4, k);
            memory4 = _mm_mul_ps(memory4, l);
            memory4 = _mm_add_ps(memory4, temp);
            _mm_storeu_ps(memory + i, memory4);
        }
    }
#endif


template <typename T>
inline std::shared_ptr <LookupTableParams<T>> makeLPFDirectFilterLookup()
{
    std::shared_ptr <LookupTableParams<T>> params = std::make_shared< LookupTableParams<T>>();

    // Fill the table with values go found by trial and error.
    // Interpolate most of the odd values

    LookupTable<T>::init(*params, 10, 0, 1, [](double x) {
        T y = 0;
        if (AudioMath::closeTo(x, 0, .001)) {
            y = T(.4);
        } else if (AudioMath::closeTo(x, .1, .001)) {
            y = T(.4);
        } else if (AudioMath::closeTo(x, .2, .001)) {
            y = T(.044);
        } else if (AudioMath::closeTo(x, .3, .001)) {
            y = T(.044 + .016)/2;
        } else if (AudioMath::closeTo(x, .4, .001)) {
            y = T(.019);
        } else if (AudioMath::closeTo(x, .5, .001)) {
            y = T(.019 + .0019) / 2;
        } else if (AudioMath::closeTo(x, .6, .001)) {
            y = T(.0019);
        } else if (AudioMath::closeTo(x, .7, .001)) {
            y = T(.0019 + .00011) / 2;
        } else if (AudioMath::closeTo(x, .8, .001)) {
            y = T(.00011);
        } else if (AudioMath::closeTo(x, .9, .001)) {
            y = T(.00005);
        } else if (AudioMath::closeTo(x, 1, .001)) {
            y = T(.00002);
        } else if (AudioMath::closeTo(x, 1.1, .001)) {
            y = T(.00002);
        } else {
            assert(false);
        }
        return y;
    });
    return params;
}
