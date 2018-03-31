#pragma once

#include "LookupTable.h"

template<typename T>
class LookupTableFactory
{
public:
    /**
    * Factory methods for exp base 2
    * domain = 0..10
    * range = 20..20k (for now). but should be .001 to 1.0?
    */
    static void makeExp2(LookupTableParams<T>& params);
    static double expYMin()
    {
        return  4;
    }
    static double expYMax()
    {
        return  40000;
    }
    static double expXMin()
    {
        return  std::log2(expYMin());
    }
    static double expXMax()
    {
        return  std::log2(expYMax());
    }
};


template<typename T>
inline void LookupTableFactory<T>::makeExp2(LookupTableParams<T>& params)
{
    // 128 not enough for one cent
    const int bins = 256;
    const T xMin = (T) std::log2(expYMin());
    const T xMax = (T) std::log2(expYMax());
    assert(xMin < xMax);
    LookupTable<T>::init(params, bins, xMin, xMax, [](double x) {
        return std::pow(2, x);
        });
}