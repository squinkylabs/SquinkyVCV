
#include "Streamer.h"

#include <assert.h>
#include <stdio.h>

#include "CubicInterpolator.h"

float_4 Streamer::step() {
    float_4 ret;

    #if 0   // try mono
    for (int channel = 0; channel < 4; ++channel) {
        ChannelData& cd = channels[channel];
        float f = cd.transposeEnabled ? stepTranspose(cd) : stepNoTranspose(cd);
        ret[channel] = f;
    }
    #else
        ChannelData& cd = channels[0];
        float f = cd.transposeEnabled ? stepTranspose(cd) : stepNoTranspose(cd);
        //ret[channel] = f;
        ret = float_4(f);
    #endif
    return ret;
}

#ifdef _TRACK_MAX
static float maxSamp = 0;
#endif

float Streamer::stepTranspose(ChannelData& cd) {
    float ret = 0;
    assert(cd.transposeEnabled);

    if (CubicInterpolator<float>::canInterpolate(cd.curFloatSampleOffset, cd.frames)) {
        // if (cd.curFloatSampleOffset < (cd.frames)) {
        assert(cd.arePlaying);
        ret = CubicInterpolator<float>::interpolate(cd.data, cd.curFloatSampleOffset);
        cd.curFloatSampleOffset += cd.transposeMultiplier;
    }

    if (!CubicInterpolator<float>::canInterpolate(cd.curFloatSampleOffset, cd.frames)) {
        cd.arePlaying = false;
    }
#ifdef _TRACK_MAX
    if (std::abs(ret) > maxSamp) {
        printf(" trans new max = %f\n", ret); fflush(stdout);
        maxSamp = std::abs(ret);
    }
#endif
    //  printf("streamer trans returning s=%f vol=%f\n", ret, cd.vol);
    return ret;
    // return ret * cd.vol;
}

float Streamer::stepNoTranspose(ChannelData& cd) {
    float ret = 0;
    assert(!cd.transposeEnabled);

    // we don't need this compare, could be arePlaying
    if (cd.curIntegerSampleOffset < (cd.frames)) {
        assert(cd.arePlaying);
        ret = cd.data[cd.curIntegerSampleOffset];
        ++cd.curIntegerSampleOffset;
    }
    if (cd.curIntegerSampleOffset >= cd.frames) {
        cd.arePlaying = false;
    }
#ifdef _TRACK_MAX
    if (std::abs(ret) > maxSamp) {
        printf("no trans new max = %f\n", ret);
        fflush(stdout);
        maxSamp = std::abs(ret);
    }
#endif
    // printf("streamer no trans returning s=%f vol=%f\n", ret, cd.vol);
    return ret;
    // return ret * cd.vol;
}

void Streamer::mute(int channel) {
    assert(channel < 4);
    printf("player just mutes channel %d\n", channel);
    channels[channel].vol = 0;
}

bool Streamer::canPlay(int channel) {
    assert(channel < 4);
    const ChannelData& cd = channels[channel];
    return bool(cd.data && cd.arePlaying);
}

void Streamer::setSample(int channel, float* d, int f) {
    assert(channel < 4);
    ChannelData& cd = channels[channel];

    cd.data = d;
    cd.frames = f;
    cd.arePlaying = true;
    cd.curIntegerSampleOffset = 0;
    cd.curFloatSampleOffset = 1;  // start one past, to allow for interpolator padding
    cd.vol = 1;
}
void Streamer::setTranspose(int channel, bool doTranspose, float amount) {
    // printf("streamer trans ch=%d amd=%f\n", channel, amount); fflush(stdout);
    assert(channel < 4);
    ChannelData& cd = channels[channel];
    cd.transposeEnabled = doTranspose;
    cd.transposeMultiplier = amount;
}
