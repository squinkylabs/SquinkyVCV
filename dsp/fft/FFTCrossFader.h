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
    /**
     * The size of the crossfade, in samples
     */
    const int crossfadeSamples;

    /**
     * current playhead, relative to start of buffer
     */
    int curPlayOffset0 = 0;
    int curPlayOffset1 = 0;


    NoiseMessage* dataFrames[3] = {nullptr, nullptr, nullptr};
};