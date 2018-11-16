#pragma once

#include "AudioMath.h"
#include <assert.h>
#include <cmath>

/**
 * initial CPU = 3.0, 54.1 change freq every sample
 */
template <int N>
class MultiLag
{
public:
    /**
     * attack and release specified as normalized frequency (LPF equivalent)
     */
    void setAttack(float);
    void setRelease(float);
    void step(const float * buffer);
    float get(int index) const
    {
        assert(index < N);
        return memory[index];
    }
private:
    float memory[N] = {0};

    float lAttack = 0;
    float kAttack = 0;
    float lRelease = 0;
    float kRelease = 0;
};

template <int N>
inline void MultiLag<N>::setAttack(float fs)
{
    assert(fs > 00 && fs < .5);
    kAttack = float(1.0 - (std::exp(-2.0 * AudioMath::Pi * fs)));
    lAttack = float(1.0 - kAttack);
}

template <int N>
inline void MultiLag<N>::setRelease(float fs)
{
    assert(fs > 00 && fs < .5);
    kRelease = float(1.0 - (std::exp(-2.0 * AudioMath::Pi * fs)));
    lRelease = float(1.0 - kRelease);
}

/**
 * z = _z * _l + _k * x;
 */
template <int N>
inline void MultiLag<N>::step(const float * input)
{
    for (int i = 0; i < N; ++i) {
        if (input[i] > memory[i]) {
            memory[i] = memory[i] * lAttack + kAttack * input[i];
        } else {
            memory[i] = memory[i] * lRelease + kRelease * input[i];
        }
    }
}

/******************************************************************************************************/

/**
 * initial CPU = 2.3, 28.1 change freq every sample
 */
template <int N>
class MultiLPF
{
public:
    // normalized cutoff freq
    void setCutoff(float);

    void step(const float * buffer);

    float get(int index) const
    {
        assert(index < N);
        return memory[index];
    }

private:
    float memory[N] = {0};

    float l = 0;
    float k = 0;
};


template <int N>
inline void MultiLPF<N>::setCutoff(float fs)
{
    assert(fs > 00 && fs < .5);
    k = float(1.0 - (std::exp(-2.0 * AudioMath::Pi * fs)));
    l = float(1.0 - k);
}

/**
 * z = _z * _l + _k * x;
 */
template <int N>
inline void MultiLPF<N>::step(const float * input)
{
    for (int i = 0; i < N; ++i) {
        memory[i] = memory[i] * l + k * input[i];
    }
}
