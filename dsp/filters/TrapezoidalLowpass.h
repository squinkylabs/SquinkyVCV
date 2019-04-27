#pragma once

#include "NonUniformLookupTable.h"

/**
 * a one pole lowpass filter
 * has 6db less control voltage feedthrough than standard lpf
 */
template <typename T>
class TrapezoidalLowpass
{
public:
    T run(T g2, T input);

#if 0
    void setG(T x)
    {
        _g = x;
        _g2 = _g / (1 + _g);
    }
#endif
    static T legacyCalcG2(T g);
private:
    T _z = 0;
  //  T _g = 0;
 //   T _g2 = 0;
};

template <typename T>
inline T TrapezoidalLowpass<T>::legacyCalcG2(T g)
{
    return g / (1 + g);
}

template <typename T>
inline T TrapezoidalLowpass<T>::run(T vin, T _g2)
{
    const T temp = (vin - _z) * _g2;
    const T output = temp + _z;
    _z = output + temp;
    return output;
}

/*
f / fs = 0.309937, g2 = 0.600000
f / fs = 0.202148, g2 = 0.428571
f / fs = 0.112793, g2 = 0.272727
f / fs = 0.058472, g2 = 0.157895
f / fs = 0.029602, g2 = 0.085714
f / fs = 0.014893, g2 = 0.044776
f / fs = 0.007446, g2 = 0.022901
f / fs = 0.003723, g2 = 0.011583
f / fs = 0.001892, g2 = 0.005825
f / fs = 0.000977, g2 = 0.002921
f / fs = 0.000488, g2 = 0.001463
f / fs = 0.000244, g2 = 0.000732
f / fs = 0.000122, g2 = 0.000366
f / fs = 0.000061, g2 = 0.000183
*/

/**
 * factory for fast lookup for LPF 'l' param.
 * This version is not particularly accurate, and is mostly
 * accurate in the low freq.
 */
template <typename T>
inline std::shared_ptr<NonUniformLookupTableParams<T>> makeTrapFilter_Lookup()
{
    std::shared_ptr<NonUniformLookupTableParams<T>> ret = std::make_shared<NonUniformLookupTableParams<T>>();
    NonUniformLookupTable<T>::addPoint(*ret, 0.309937, 0.600000);
    NonUniformLookupTable<T>::addPoint(*ret, 0.202148, 0.428571);
    NonUniformLookupTable<T>::addPoint(*ret, 0.112793, 0.272727);
    NonUniformLookupTable<T>::addPoint(*ret, 0.058472, 0.157895);
    NonUniformLookupTable<T>::addPoint(*ret, 0.029602, 0.085714);
    NonUniformLookupTable<T>::addPoint(*ret, 0.014893, 0.044776);
    NonUniformLookupTable<T>::addPoint(*ret, 0.007446, 0.022901);
    NonUniformLookupTable<T>::addPoint(*ret, 0.003723, 0.011583);
    NonUniformLookupTable<T>::addPoint(*ret, 0.001892, 0.005825);
    NonUniformLookupTable<T>::addPoint(*ret, 0.000977, 0.002921);
    NonUniformLookupTable<T>::addPoint(*ret, 0.000488, 0.001463);
    NonUniformLookupTable<T>::addPoint(*ret, 0.000244, 0.000732);
    NonUniformLookupTable<T>::addPoint(*ret, 0.000122, 0.000366);
    NonUniformLookupTable<T>::addPoint(*ret, 0.000061, 0.000183);

    NonUniformLookupTable<T>::finalize(*ret);
    return ret;
}

