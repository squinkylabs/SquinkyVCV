#pragma once

#include <assert.h>

template <typename T>
class CubicInterpolator
{
public:
    /**
     * don't call interpolate if canInterpolate returns false
     */
    static bool canInterpolate(T offset, unsigned int totalSize);
    static T interpolate(const T* data, T offset);
private:
    static unsigned int getIntegerPart(T);
    static T getFloatPart(T);
};

template <typename T>
inline bool CubicInterpolator<T>::canInterpolate(T offset, unsigned int totalSize)
{
    const unsigned int index = getIntegerPart(offset);
    return (offset > 0 && offset < (totalSize - 2));
}

#if defined(_MSC_VER)
//#define __attribute__(x)

#pragma warning (push)
#pragma warning ( disable: 4244 4305 )
#define NOMINMAX
#endif

template <typename T>
inline T CubicInterpolator<T>::interpolate(const T* data, T offset)
{
    unsigned int delayTimeSamples = getIntegerPart(offset);
   // const double x = delayTime - delayTimeSamples;
    const double x = getFloatPart(offset);


    const T y0 = data[delayTimeSamples - 1];
    const T y1 = data[delayTimeSamples];
    const T y2 = data[delayTimeSamples + 1];
    const T y3 = data[delayTimeSamples + 2];

#ifdef _LOG
    printf("dt=%.2f, dts=%d x=%.2f ", delayTime, delayTimeSamples, x);
    printf("y0=%.2f y1=%.2f y2=%.2f y3=%.2f\n", y0, y1, y2, y3);
#endif
 
    const double x0 = -1.0;
    const double x1 = 0.0;
    const double x2 = 1.0;
    const double x3 = 2.0;
    assert(x >= x1);
    assert(x <= x2);

    T dRet = -(1.0 / 6.0) * y0 * (x - x1) * (x - x2) * (x - x3);
    dRet += (1.0 / 2.0)* y1*(x - x0) * (x - x2) * (x - x3);
    dRet += (-1.0 / 2.0)*y2*(x - x0) * (x - x1) * (x - x3);
    dRet += (1.0 / 6.0) * y3*(x - x0) * (x - x1) * (x - x2);

    return dRet;
}

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

template <typename T>
unsigned int CubicInterpolator<T>::getIntegerPart(T fpOffset)
{
    const unsigned int uintOffset  = (unsigned int) fpOffset;
    return uintOffset;
}

template <typename T>
T CubicInterpolator<T>::getFloatPart(T input)
{
    const T intPart = getIntegerPart(input);
    const T x = input - intPart;
    return x;
}

