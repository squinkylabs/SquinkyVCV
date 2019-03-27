#pragma once

#include "LookupTable.h"
#include "SqMath.h"

#ifndef _CLAMP
#define _CLAMP
namespace std {
    inline float clamp(float v, float lo, float hi)
    {
        assert(lo < hi);
        return std::min(hi, std::max(v, lo));
    }
}
#endif


// TODO: this class should not be templatized. the functions should
template<typename T>
class LookupTableFactory
{
public:
    /**
     * domain (x) = -1 .. +1
     */
    static void makeBipolarAudioTaper(LookupTableParams<T>& params);


    static void makeMixerPanL(LookupTableParams<T>& params);
    static void makeMixerPanR(LookupTableParams<T>& params);

    /**
     * domain (x) = 0..1
     */
    static void makeAudioTaper(LookupTableParams<T>& params);

    static double audioTaperKnee()
    {
        return -24;
    }
    /**
    * Factory methods for exp base 2
    * domain = 0..10
    * range = 20..20k (for now). but should be .001 to 1.0?
    */
    static void makeExp2(LookupTableParams<T>& params);
    static double exp2YMin()
    {
        return  4;
    }
    static double exp2YMax()
    {
        return  40000;
    }
    static double exp2XMin()
    {
        return  std::log2(exp2YMin());
    }
    static double exp2XMax()
    {
        return  std::log2(exp2YMax());
    }

    static void makeExp2ExLow(LookupTableParams<T>& params);
    static double exp2ExLowYMin()
    {
        return  2;
    }

    static double exp2ExLowYMax()
    {
        return  400;
    }
    static double exp2ExLowXMin()
    {
        return  std::log2(exp2ExLowYMin());
    }
    static double exp2ExLowXMax()
    {
        return  std::log2(exp2ExLowYMax());
    }

    static void makeExp2ExHigh(LookupTableParams<T>& params);
    static double exp2ExHighYMin()
    {
        return  400;
    }
    static double exp2ExHighYMax()
    {
        return  20000;
    }
    static double exp2ExHighXMin()
    {
        return  std::log2(exp2ExHighYMin());
    }

    static double exp2ExHighXMax()
    {
        return  std::log2(exp2ExHighYMax());
    }
};

static inline float _PanL(float balance, float cv)
{ // -1...+1
    float p, inp;
    inp = balance + cv / 5;
    p = M_PI * (std::clamp(inp, -1.0f, 1.0f) + 1) / 4;
    return ::cos(p);
}

static inline float _PanR(float balance, float cv)
{
    float p, inp;
    inp = balance + cv / 5;
    p = M_PI * (std::clamp(inp, -1.0f, 1.0f) + 1) / 4;
    return ::sin(p);
}

template<typename T>
inline void LookupTableFactory<T>::makeMixerPanL(LookupTableParams<T>& params)
{
    const int bins = 16;
    const T xMin = -1;
    const T xMax = 1;
    assert(xMin < xMax);
    LookupTable<T>::init(params, bins, xMin, xMax, [](double x) {
        return _PanL(x, 0);
        });
}

template<typename T>
inline void LookupTableFactory<T>::makeMixerPanR(LookupTableParams<T>& params)
{
    const int bins = 16;
    const T xMin = -1;
    const T xMax = 1;
    assert(xMin < xMax);
    LookupTable<T>::init(params, bins, xMin, xMax, [](double x) {
        return _PanR(x, 0);
        });
}

template<typename T>
inline void LookupTableFactory<T>::makeExp2(LookupTableParams<T>& params)
{
    // 256 enough for one cent
    const int bins = 256;
    const T xMin = (T) std::log2(exp2YMin());
    const T xMax = (T) std::log2(exp2YMax());
    assert(xMin < xMax);
    LookupTable<T>::init(params, bins, xMin, xMax, [](double x) {
        return std::pow(2, x);
        });
}

// hit specs with 256 / 128 @200 crossover
// try 800 ng - need 256/256
// 400 need 256 / 128
template<typename T>
inline void LookupTableFactory<T>::makeExp2ExHigh(LookupTableParams<T>& params)
{
    const int bins = 512;
    const T xMin = (T) std::log2(exp2ExHighYMin());
    const T xMax = (T) std::log2(exp2ExHighYMax());
    assert(xMin < xMax);
    LookupTable<T>::init(params, bins, xMin, xMax, [](double x) {
        return std::pow(2, x);
        });
}

template<typename T>
inline void LookupTableFactory<T>::makeExp2ExLow(LookupTableParams<T>& params)
{
    const int bins = 256;
    const T xMin = (T) std::log2(exp2ExLowYMin());
    const T xMax = (T) std::log2(exp2ExLowYMax());
    assert(xMin < xMax);
    LookupTable<T>::init(params, bins, xMin, xMax, [](double x) {
        return std::pow(2, x);
        });
}

template<typename T>
inline void LookupTableFactory<T>::makeBipolarAudioTaper(LookupTableParams<T>& params)
{
    const int bins = 32;
    std::function<double(double)> audioTaper = AudioMath::makeFunc_AudioTaper(audioTaperKnee());
    const T xMin = -1;
    const T xMax = 1;
    LookupTable<T>::init(params, bins, xMin, xMax, [audioTaper](double x) {
        return (x >= 0) ? audioTaper(x) : -audioTaper(-x);
        });

}

template<typename T>
inline void LookupTableFactory<T>::makeAudioTaper(LookupTableParams<T>& params)
{
    const int bins = 32;
    std::function<double(double)> audioTaper = AudioMath::makeFunc_AudioTaper(audioTaperKnee());
    const T xMin = 0;
    const T xMax = 1;
    LookupTable<T>::init(params, bins, xMin, xMax, [audioTaper](double x) {
        return audioTaper(x);
        });

}