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
     * current playhead, relative to start of each buffer
     */
    int curPlayOffset[2] = {0, 0};


    NoiseMessage* dataFrames[2] = {nullptr, nullptr};

    /** Advance the play offset,
     * wrap on overflow.
     */
    void advance(int index);
};