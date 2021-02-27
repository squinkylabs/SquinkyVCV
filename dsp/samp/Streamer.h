
#pragma once

#include "SimdBlocks.h"

/**
 * This is a four channel streamer.
 * Streamer is the thing that plays out a block of samples, possibly at an
 * altered rate.
 */
class Streamer {
public:
    void setSample(int chan, float* data, int frames);
    void setTranspose(int chan, bool doTranspoe, float amount);
    bool canPlay(int chan);

    void setGain(int chan, float gain);


    float_4 step();

    /** On each channel,
     * how many samples are left?
     */
    float_4 audioSamplesRemaining() const;

    void _assertValid();

public:
    class ChannelData {
    public:
        float* data = nullptr;
        int frames = 0;

        float vol = 1;  // this will go away when we have envelopes

        int curIntegerSampleOffset = 0;
        bool arePlaying = false;
        float curFloatSampleOffset = 0;
        bool transposeEnabled = false;
        float transposeMultiplier = 1;
        float gain = 1;

        void _dump() const;
    };
    ChannelData channels[4];

    float stepNoTranspose(ChannelData&);
    float stepTranspose(ChannelData&);
};
