#pragma once

#include <assert.h>

#include "SqLog.h"

template <typename T>
class CubicInterpolator {
public:
    /**
     * don't call array interpolate if canInterpolate returns false
     */
    static bool canInterpolate(T offset, unsigned int totalSize);
    static T interpolate(const T* data, T offset);
    static T interpolate(T offset, T y0, T y1, T y2, T y3);
    static unsigned int getIntegerPart(T);

private:
    static T getFloatPart(T);
};


// Should more properly be called canInterpolateInPlace
template <typename T>
inline bool CubicInterpolator<T>::canInterpolate(T offset, unsigned int totalSize) {
    // const unsigned int index = getIntegerPart(offset);
    return (offset >= 1 && offset < (totalSize - 2));
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244 4305)
#define NOMINMAX
#endif

template <typename T>
inline T CubicInterpolator<T>::interpolate(T offset, T y0, T y1, T y2, T y3) {
    const double x0 = -1.0;
    const double x1 = 0.0;
    const double x2 = 1.0;
    const double x3 = 2.0;

    const double x = getFloatPart(offset);
    assert(x >= x1);
    assert(x <= x2);

    T dRet = -(1.0 / 6.0) * y0 * (x - x1) * (x - x2) * (x - x3);
    dRet += (1.0 / 2.0) * y1 * (x - x0) * (x - x2) * (x - x3);
    dRet += (-1.0 / 2.0) * y2 * (x - x0) * (x - x1) * (x - x3);
    dRet += (1.0 / 6.0) * y3 * (x - x0) * (x - x1) * (x - x2);
    return dRet;
}
template <typename T>
inline T CubicInterpolator<T>::interpolate(const T* data, T offset) {
   // SQINFO("cubic int : int ofset=%f", offset );
    assert(offset >= 0);
    unsigned int delayTimeSamples = getIntegerPart(offset);
    assert(delayTimeSamples >= 1);

    //   const double x = getFloatPart(offset);
    const T y0 = data[delayTimeSamples - 1];
    const T y1 = data[delayTimeSamples];
    const T y2 = data[delayTimeSamples + 1];
    const T y3 = data[delayTimeSamples + 2];
    return interpolate(offset, y0, y1, y2, y3);
}

#if 0   // before divide up:
template <typename T>
inline T CubicInterpolator<T>::interpolate(const T* data, T offset) {
#if 1
    unsigned int delayTimeSamples = getIntegerPart(offset);
    const double x = getFloatPart(offset);

    const T y0 = data[delayTimeSamples - 1];
    const T y1 = data[delayTimeSamples];
    const T y2 = data[delayTimeSamples + 1];
    const T y3 = data[delayTimeSamples + 2];

    const double x0 = -1.0;
    const double x1 = 0.0;
    const double x2 = 1.0;
    const double x3 = 2.0;
    assert(x >= x1);
    assert(x <= x2);

    T dRet = -(1.0 / 6.0) * y0 * (x - x1) * (x - x2) * (x - x3);
    dRet += (1.0 / 2.0) * y1 * (x - x0) * (x - x2) * (x - x3);
    dRet += (-1.0 / 2.0) * y2 * (x - x0) * (x - x1) * (x - x3);
    dRet += (1.0 / 6.0) * y3 * (x - x0) * (x - x1) * (x - x2);

#else
   
    T dRet = 0;
#endif

    return dRet;
}

#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

template <typename T>
unsigned int CubicInterpolator<T>::getIntegerPart(T fpOffset) {
    const unsigned int uintOffset = (unsigned int)fpOffset;
    return uintOffset;
}

template <typename T>
T CubicInterpolator<T>::getFloatPart(T input) {
    const T intPart = getIntegerPart(input);
    const T x = input - intPart;
    return x;
}
