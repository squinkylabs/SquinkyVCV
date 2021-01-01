
#pragma once

#include "SimdBlocks.h"

class Streamer {
public:
    void setSample(int chan, float* data, int frames);
    void setTranspose(int chan, bool doTranspoe, float amount);
    bool canPlay(int chan);
    void mute(int chan);

    // TODO: float 4?
    float_4 step();

public:
    class ChannelData {
    public:
        float* data = nullptr;
        int frames = 0;

        float vol = 1;

        int curIntegerSampleOffset = 0;
        bool arePlaying = false;
        float curFloatSampleOffset = 0;
        bool areTransposing = false;
        bool transposeEnabled = false;
        float transposeMultiplier = 1;
    };
    ChannelData channels[4];

    float stepNoTranspose(ChannelData&);
    float stepTranspose(ChannelData&);
};
