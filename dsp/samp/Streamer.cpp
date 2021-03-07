
#include "Streamer.h"

#include <assert.h>
#include <stdio.h>

#include <algorithm>

#include "CubicInterpolator.h"
#include "SqLog.h"

float_4 Streamer::step(float_4 fm, bool fmEnabled) {
    float_4 ret;
    // SQINFO("St:Step %d, %s", fmEnabled, toStr(fm).c_str());
    for (int channel = 0; channel < 4; ++channel) {
        ChannelData& cd = channels[channel];

        if (cd.data) {
            const bool doInterp = cd.transposeEnabled || fmEnabled;
            // SQWARN("do in = %d fm=%d", doInterp, fmEnabled);
            float scalarData = doInterp ? stepTranspose(cd, fm[channel]) : stepNoTranspose(cd);

            const float acceptable = 1.1f;
            if (scalarData > acceptable || scalarData < -acceptable) {
                SQWARN("bad sample value from step %f", scalarData);
                SQWARN("pos = %, %df", cd.curFloatSampleOffset, cd.curIntegerSampleOffset);
            }
            assert(scalarData <= acceptable);
            assert(scalarData >= -acceptable);
            scalarData *= cd.gain;
            ret[channel] = scalarData;
        } else {
            // now we called in this state sometimes. Not sure why,
            // but it certainly seems reasonable to handle it.
            ret[channel] = 0;
        }
    }
    return ret;
}

float Streamer::stepTranspose(ChannelData& cd, float lfm) {
    float ret = 0;
 //   assert(cd.transposeEnabled);
  //  assert(lfm == 0);

  //  SQINFO("St:Step% %f", lfm);

    if (CubicInterpolator<float>::canInterpolate(float(cd.curFloatSampleOffset), cd.frames)) {
        if (!cd.arePlaying) {
            SQWARN("why are we playing a streamer not started? pl=%d, fo=%f, io=%d", cd.arePlaying, cd.curFloatSampleOffset, cd.curIntegerSampleOffset);
            assert(false);
        }

        ret = CubicInterpolator<float>::interpolate(cd.data, float(cd.curFloatSampleOffset));
        cd.curFloatSampleOffset += cd.transposeMultiplier;
        cd.curFloatSampleOffset += lfm;
        cd.curFloatSampleOffset = std::max<float>(2.f, cd.curFloatSampleOffset);
    }

    if (!CubicInterpolator<float>::canInterpolate(float(cd.curFloatSampleOffset), cd.frames)) {
        cd.arePlaying = false;
    }

    return ret * cd.vol;
}

float Streamer::stepNoTranspose(ChannelData& cd) {
    // SQINFO("step no tran");
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
    cd.arePlaying = true;  // this variable doesn't mean much, but???
    cd.curIntegerSampleOffset = 0;
    cd.curFloatSampleOffset = 1;  // start one past, to allow for interpolator padding
    cd.vol = 1;
}

void Streamer::clearSamples() {
    SQINFO("Streamer::clearSamples()");
    for (int channel = 0; channel < 4; ++channel) {
        clearSamples(channel);
    }
}

void Streamer::clearSamples(int channel) {
    setSample(channel, nullptr, 0);
}

void Streamer::setTranspose(int channel, bool doTranspose, float amount) {
    SQINFO("streamer trans ch=%d amt=%f\n", channel, amount);
    assert(channel < 4);
    ChannelData& cd = channels[channel];
    cd.transposeEnabled = doTranspose;
    cd.transposeMultiplier = amount;
}

void Streamer::ChannelData::_dump() const {
    SQINFO("dumping %p", this);
    SQINFO("vol=%f, te=%d tm=%f g = %f", vol, transposeEnabled, transposeMultiplier, gain);
}

void Streamer::_assertValid() {
    for (int channel = 0; channel < 4; ++channel) {
        ChannelData& cd = channels[channel];

        if (cd.transposeEnabled) {
            SQINFO("finihs overrun checking!");
            if (cd.arePlaying)
                assert(CubicInterpolator<float>::canInterpolate(float(cd.curFloatSampleOffset), cd.frames));

        } else {
            // these can be equal, if we play past end
            if (cd.arePlaying) {
                assert(cd.curIntegerSampleOffset < cd.frames);
            } else {
                assert(cd.curIntegerSampleOffset <= cd.frames);
            }
        }
    }
}