#pragma once

#include "NonUniformLookupTable.h"

#include <memory>

/**
 * a one pole lowpass filter
 * has 6db less control voltage feedthrough than standard lpf
 */
template <typename T>
class TrapezoidalLowpass
{
public:
    T run(T g2, T input);
    static T legacyCalcG2(T g);
private:
    T _z = 0;
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
    NonUniformLookupTable<T>::addPoint(*ret, T(0.309937), T(0.600000));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.202148), T(0.428571));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.112793), T(0.272727));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.058472), T(0.157895));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.029602), T(0.085714));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.014893), T(0.044776));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.007446), T(0.022901));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.003723), T(0.011583));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.001892), T(0.005825));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.000977), T(0.002921));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.000488), T(0.001463));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.000244), T(0.000732));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.000122), T(0.000366));
    NonUniformLookupTable<T>::addPoint(*ret, T(0.000061), T(0.000183));

    NonUniformLookupTable<T>::finalize(*ret);
    return ret;
}


template <typename T>
class TrapezoidalHighpass
{
public:
    T run(T g2, T input);
private:
    T _z = 0;
};

template <typename T>
inline T TrapezoidalHighpass<T>::run(T vin, T _g2)
{
    const T temp = (vin - _z) * _g2;
    const T outputLP = temp + _z;
    _z = outputLP + temp;
    return vin - outputLP;
}
