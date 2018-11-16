#pragma once


#include "AudioMath.h"
#include "NonUniformLookupTable.h"

#include <assert.h>
#include <memory>

template <typename T>
class LowpassFilterState
{
public:
    T z=0;
};

template <typename T>
class LowpassFilterParams
{
public:
    T k=0;
    T l=0;
};

template <typename T>
class LowpassFilter
{
public:
    /**
     * fs is normalize frequency
     */
    static void setCutoff(LowpassFilterParams<T>& params, T fs);

    static T computeK(T fs);
    static T computeL(T k);

    static T run(T input, LowpassFilterState<T>& state, const LowpassFilterParams<T>& params);
};

template <typename T>
inline  T LowpassFilter<T>::computeK(T fs)
{
    return  T(1.0 - (std::exp(-2.0 * AudioMath::Pi * fs)));
}

template <typename T>
inline  T LowpassFilter<T>::computeL(T k)
{
    return T(1.0 - k);
}


template <typename T>
inline  void LowpassFilter<T>::setCutoff(LowpassFilterParams<T>& params, T fs)
{
    assert(fs > 00 && fs < .5);
    params.k = T(1.0 - (std::exp(-2.0 * AudioMath::Pi * fs)));
    params.l = T(1.0 - params.k);
}


/*
void go_dbl(double x)
{
_z = _z * _l + _k * x;
}
*/
template <typename T>
inline T LowpassFilter<T>::run(T input, LowpassFilterState<T>& state, const LowpassFilterParams<T>& params)
{
    state.z = state.z * params.l + params.k * input;
    return state.z;
}

/**
 * factory for fast lookup for LPF 'k' param.
 * This version is not particularly accurate, and is mostly
 * accurate in the low freq.
 */
template <typename T>
inline std::shared_ptr<NonUniformLookupTableParams<T>> makeLPFilterLookup()
{
    std::shared_ptr<NonUniformLookupTableParams<T>> ret = std::make_shared<NonUniformLookupTableParams<T>>();

    T freqs[] = {22000, 1000, 100, 10, 1, .1f};
    int numFreqs = sizeof(freqs) / sizeof(T);
    
    for (int i = 0; i < numFreqs; ++i) {
        T fs = freqs[i] / 44100.f;
        T k = LowpassFilter<T>::computeK(fs);
        NonUniformLookupTable<T>::addPoint(*ret, fs, k);
    }
    NonUniformLookupTable<T>::finalize(*ret);
    return ret;
}