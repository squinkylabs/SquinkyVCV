
#include "Streamer.h"

#include <assert.h>
#include <stdio.h>

#include <algorithm>

#include "CubicInterpolator.h"
#include "SqLog.h"

float_4 Streamer::step() {
    float_4 ret;

    for (int channel = 0; channel < 4; ++channel) {
        ChannelData& cd = channels[channel];

        // I guess we can get called when this it true. do we even care? is the variable useful?
        // assert(cd.arePlaying);
        float f = cd.transposeEnabled ? stepTranspose(cd) : stepNoTranspose(cd);
        assert(f <= 1);
        assert(f >= -1);
        f *= cd.gain;
        ret[channel] = f;
    }
    return ret;
}

float Streamer::stepTranspose(ChannelData& cd) {
    float ret = 0;
    assert(cd.transposeEnabled);

    if (CubicInterpolator<float>::canInterpolate(cd.curFloatSampleOffset, cd.frames)) {
        // if (cd.curFloatSampleOffset < (cd.frames)) {
        //  assert(cd.arePlaying);
        if (!cd.arePlaying) {
            SQWARN("why are we playing a streamer not started? pl=%d, fo=%f, io=%d", cd.arePlaying, cd.curFloatSampleOffset, cd.curIntegerSampleOffset);
            assert(false);
        }

        ret = CubicInterpolator<float>::interpolate(cd.data, cd.curFloatSampleOffset);
        cd.curFloatSampleOffset += cd.transposeMultiplier;
    }

    if (!CubicInterpolator<float>::canInterpolate(cd.curFloatSampleOffset, cd.frames)) {
        cd.arePlaying = false;
    }

    return ret * cd.vol;
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
        //  SQINFO("play to end");
        cd.arePlaying = false;
    }

    return ret * cd.vol;
}

bool Streamer::canPlay(int channel) {
    assert(channel < 4);
    const ChannelData& cd = channels[channel];
    return bool(cd.data && cd.arePlaying);
}

void Streamer::setGain(int channel, float gain) {
    ChannelData& cd = channels[channel];
    cd.gain = gain;
}

void Streamer::setSample(int channel, float* d, int f) {
    assert(channel < 4);
    ChannelData& cd = channels[channel];

    // temporary validity test
#ifndef NDEBUG
    // SQINFO("st::setSample(%d) siz=%d", channel, f);
    for (int i = 0; i < f; ++i) {
        const float x = d[i];
        assert(x <= 1);
        assert(x >= -1);
    }
#endif
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

float_4 Streamer::audioSamplesRemaining() const {
    float_4 ret(0);
    for (int channel = 0; channel < 4; ++channel) {
        const ChannelData& cd = channels[channel];

        unsigned curPosition = std::max<unsigned>(cd.curIntegerSampleOffset, (unsigned)cd.curFloatSampleOffset);
        int framesRemaining = cd.frames - curPosition;
        // assert(framesRemaining >= 0);
        if (framesRemaining < 0) {
            SQWARN("frames rem =%d", framesRemaining);

            if (framesRemaining <= -10) {
                SQINFO("we have %d frames, pos = %d", cd.frames, curPosition);
                cd._dump();

                // I think assert valid will catch this now.
             //   assert(framesRemaining > -10);
            }
        }
        framesRemaining = std::max(framesRemaining, 0);

        // kind of sleazy forcing this into a float, but it will be fine.
        ret[channel] = float(framesRemaining);
    }

    return ret;
}

void Streamer::ChannelData::_dump() const {
    SQINFO("dumping %p", this);
    SQINFO("vol=%f, te=%d tm=%f g = %f", vol, transposeEnabled, transposeMultiplier, gain);
}

void Streamer::_assertValid() {
#if 1
    for (int channel = 0; channel < 4; ++channel) {
        ChannelData& cd = channels[channel];

        if (cd.transposeEnabled) {
            SQINFO("finihs overrun checking!");
            if (cd.arePlaying)
                assert(CubicInterpolator<float>::canInterpolate(cd.curFloatSampleOffset, cd.frames));

        }
        else {
            // these can be equal, if we play past end
            if (cd.arePlaying) {
                assert(cd.curIntegerSampleOffset < cd.frames);
            }
            else {
                assert(cd.curIntegerSampleOffset <= cd.frames);
            }
        }
    }
#endif
   

}