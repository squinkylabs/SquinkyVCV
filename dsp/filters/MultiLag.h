#pragma once

#include "AudioMath.h"
#include <assert.h>
#include <cmath>

template <int N>
class MultiLag
{
public:
    /**
     * attack and release specified as normalized frequency (LPF equivalent)
     */
    void setAttack(float)
    {

    }
    void setRelease(float)
    {

    }
    void step(const float * buffer)
    {

    }
    float get(int index) const
    {
        assert(index < N);
        return memory[index];
    }
private:
    float memory[N] = {0};
};


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
