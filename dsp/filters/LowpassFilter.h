#pragma once


#include "AudioMath.h"
#include "NonUniformLookupTable.h"

#include <assert.h>
#include <memory>

template <typename T>
class LowpassFilterState
{
public:
    T z = 0;
};

template <typename T>
class LowpassFilterParams
{
public:
    T k = 0;
    T l = 0;
};

template <typename T>
class LowpassFilter
{
public:
    /**
     * fs is normalize frequency
     */
    static void setCutoff(LowpassFilterParams<T>& params, T fs);

    // old way (remove)
    //static T computeKfromFs(T fs);
   //static T computeLfromK(T k);

    static T computeLfromFs(T fs);
    static T computeKfromL(T l);

    static T run(T input, LowpassFilterState<T>& state, const LowpassFilterParams<T>& params);
};

#if 0
template <typename T>
inline  T LowpassFilter<T>::computeKfromFs(T fs)
{
    return  T(1.0 - (std::exp(-2.0 * AudioMath::Pi * fs)));
}

template <typename T>
inline  T LowpassFilter<T>::computeLfromK(T k)
{
    return T(1.0 - k);
}

#endif

template <typename T>
inline  T LowpassFilter<T>::computeLfromFs(T fs)
{
    return  T(std::exp(-2.0 * AudioMath::Pi * fs));
}

template <typename T>
inline  T LowpassFilter<T>::computeKfromL(T l)
{
    return T(1.0 - l);
}

template <typename T>
inline  void LowpassFilter<T>::setCutoff(LowpassFilterParams<T>& params, T fs)
{
    assert(fs > 00 && fs < .5);
  //  params.k = computeKfromFs(fs);
  //  params.l = computeLfromK(params.k);
    params.l = computeLfromFs(fs);
    params.k = computeKfromL(params.l);
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
 * factory for fast lookup for LPF 'l' param.
 * This version is not particularly accurate, and is mostly
 * accurate in the low freq.
 */
template <typename T>
inline std::shared_ptr<NonUniformLookupTableParams<T>> makeLPFilterL_Lookup()
{
    std::shared_ptr<NonUniformLookupTableParams<T>> ret = std::make_shared<NonUniformLookupTableParams<T>>();

    T freqs[] = {22000, 1000, 100, 10, 1, .1f};
    int numFreqs = sizeof(freqs) / sizeof(T);

    for (int i = 0; i < numFreqs; ++i) {
        T fs = freqs[i] / 44100.f;
        T l = LowpassFilter<T>::computeLfromFs(fs);
        NonUniformLookupTable<T>::addPoint(*ret, fs, l);
    }
    NonUniformLookupTable<T>::finalize(*ret);
    return ret;
}