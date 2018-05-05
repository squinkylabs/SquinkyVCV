#pragma once

class NoiseMessage;

/**
 * This is a specialized gizmo just for fading between two
 * FFT frames
 */
class FFTCrossFader
{
public:
    FFTCrossFader(int crossfadeSamples) : crossfadeSamples(crossfadeSamples)
    {
    }
    NoiseMessage * step(float* out);
    NoiseMessage * acceptData(NoiseMessage*);
private:
    const int crossfadeSamples;
};