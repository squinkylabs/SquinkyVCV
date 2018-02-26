#pragma once

#include <cmath>
/**
 * Utilities for generating and analyzing test signals.
 *
 * All test signals are +-1volt unless otherwise specified
 */

template <typename T>
class TestSignal
{
public:
    // freq is normalized f / sr
    static void generateSin(T * output, int numSamples, T freq);
    static double getRMS(const T * signal, int numSamples);
};


template <typename T>
inline void TestSignal<T>::generateSin(T * output, int numSamples, T freq)
{
    assert(freq < .5);
    assert(freq > 0);

    double phase = 0;
    for (int i = 0; i < numSamples; ++i) {
        const double phi = phase * 2 * AudioMath::Pi;

        output[i] = T(std::sin(phi));
        phase += freq;
        // should we normalize the phase here, instead of letting it grow?
    }
}

template <typename T>
inline double TestSignal<T>::getRMS(const T * signal, int numSamples)
{
    assert(numSamples > 0);
    double sumSq = 0;
    for (int i = 0; i < numSamples; ++i) {
        sumSq += double(signal[i]) * signal[i];
    }
    return std::sqrt(sumSq / numSamples);
}