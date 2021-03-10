#pragma once

#include <assert.h>

#include <cmath>

/** Is this used anywhere?
 */

class FixedPointAccumulator {
public:
    double getFractionalPart() const {
        return fractionalPart;
    }
    int getIntegralPart() const {
        return integralPart;
    }

    double getAsDouble() const {
        return fractionalPart + integralPart;
    }
    void add(double);

    void limitMin(int);
    void setToInt(int);

private:
    int integralPart = 0;
    double fractionalPart = 0;

    void normalize();
};

inline void FixedPointAccumulator::setToInt(int i) {
    integralPart = i;
    fractionalPart = 0;
}

inline void FixedPointAccumulator::limitMin(int x) {
    if (integralPart <= x) {
        integralPart = x;
        fractionalPart = 0;
    }
}

inline void FixedPointAccumulator::normalize() {
    int i = int(std::round(std::floor(fractionalPart)));
    double f = fractionalPart - i;

    integralPart += i;
    fractionalPart = f;
    assert(f >= 0);
    assert(f <= 1);
}

inline void FixedPointAccumulator::add(double d) {
    fractionalPart += d;
    normalize();
}

///// abandoned experiment

#if 0
#pragma once

#include <assert.h>

#include "FixedPointAccumulator.h"

template <typename T>
class CubicInterpolator {
public:
    /**
     * don't call interpolate if canInterpolate returns false
     */
    static bool canInterpolate(T offset, unsigned int totalSize);
    static bool canInterpolate(const FixedPointAccumulator& offset, unsigned int totalSize);

    static T interpolate(const T* data, T offset);
    static T interpolate(const T* data, const FixedPointAccumulator& offset);

private:
    static unsigned int getIntegerPart(T);
    static T getFloatPart(T);
    static T interpolateImp(const T* data, int integralPart, double fractionalPart);
};

template <typename T>
inline bool CubicInterpolator<T>::canInterpolate(T offset, unsigned int totalSize) {
    // const unsigned int index = getIntegerPart(offset);
    return (offset > 0 && offset < (totalSize - 2));
}

template <typename T>
inline bool CubicInterpolator<T>::canInterpolate(const FixedPointAccumulator& offset, unsigned int totalSize) {
    // const unsigned int index = getIntegerPart(offset);
    return (offset.getAsDouble() > 0 && offset.getAsDouble() < (totalSize - 2));
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244 4305)
#define NOMINMAX
#endif

template <typename T>
inline T CubicInterpolator<T>::interpolate(const T* data, T offset) {
    const unsigned int delayTimeSamples = getIntegerPart(offset);
    const double x = getFloatPart(offset);
    return interpolateImp(data, delayTimeSamples, x);
}

template <typename T>
inline T CubicInterpolator<T>::interpolate(const T* data, const FixedPointAccumulator& offset) {
    const unsigned int delayTimeSamples = offset.getIntegralPart();
    const double x = offset.getFractionalPart();
    return interpolateImp(data, delayTimeSamples, x);
}

template <typename T>
inline T CubicInterpolator<T>::interpolateImp(const T* data, int integralPart, double fractionalPart) {
    //   unsigned int delayTimeSamples = getIntegerPart(offset);
    //   const double x = getFloatPart(offset);
    const int delayTimeSamples = integralPart;
    const double x = fractionalPart;

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

    return dRet;
}

#if 0
template <typename T>
inline T CubicInterpolator<T>::interpolate(const T* data, T offset) {
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

#endif

////////////// old test code ////////////////////

#include <string>

#include "SqLog.h"
class EdgeCatcher {
public:
    EdgeCatcher(float thresh, const char* l) : threshold(thresh), label(l) {}
 //   bool sample(float s, double d0, double d1) {
      bool sample(float s) {
        ++counter;
        if (isFirstValue) {
            lastValue = s;
            isFirstValue = false;
            return false;
        }
        bool r = false;
        float delta = std::abs(lastValue - s);
        if (delta > threshold) {
            if (!havePrinted) {
                SQINFO("*********************************************");
                SQINFO("jump detectd at %s size %f", label.c_str(), delta);
                SQINFO("value is %f, was %f", s, lastValue);
                SQINFO("index = %d", counter);
             //   SQINFO("do: %f -> %f", lastd0, do);
             //   SQINFO("d1: %f -> %f", lastd0, d1);
                SQINFO("*********************************************");
                havePrinted = true;
                r = true;
            }
        }
        lastValue = s;
     //   lastd0 = d0;
     //   lastd1 = d1;
        return r;
    }

private:
    float threshold;
    float lastValue = 0;
    double lastd0 = 0;
    double lastd1 = 0;
    bool isFirstValue = true;
    std::string label;
    bool havePrinted = false;
    int counter = 0;
};
