
#include "Streamer.h"

#include <assert.h>
#include <stdio.h>

#include <algorithm>

#include "CubicInterpolator.h"
#include "SqLog.h"

#define _INTERP
//#define _LOG

float_4 Streamer::step(float_4 fm, bool fmEnabled) {
    float_4 ret = 0;
    // SQINFO("St:Step %d, %s", fmEnabled, toStr(fm).c_str());
    for (int channel = 0; channel < 4; ++channel) {
        ChannelData& cd = channels[channel];

        assert(!std::isinf(cd.curFloatSampleOffset));
        assert(!std::isinf(cd.transposeMultiplier));

        // if (cd.canPlay()) {
        if (cd.data) {
            const bool doInterp = true;  // until we can figure out a way to enable it without pops, we will leave "no transpose" disabled.
            //const bool doInterp = cd.transposeEnabled || fmEnabled;

            float scalarData = doInterp ? stepTranspose(cd, fm[channel]) : stepNoTranspose(cd);

            // if we get rid of these dumb balidity checks, then our
            // unit tests can pump crazy tests data. These never go off anyway.
#if 0
            const float acceptable = 1.1f;
            if (scalarData > acceptable || scalarData < -acceptable) {
                SQWARN("bad sample value from step %f", scalarData);
                SQWARN("pos = %, %df", cd.curFloatSampleOffset, cd.curIntegerSampleOffset);
            }
            assert(scalarData <= acceptable);
            assert(scalarData >= -acceptable);
#endif
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
    assert(cd.curFloatSampleOffset >= 0);
#ifdef _LOG
    SQINFO("in stepTranspose offset=%f cd=%p", cd.curFloatSampleOffset, &cd);
#endif
    if (cd.loopActive && (cd.curFloatSampleOffset >= (cd.loopData.loop_end - 2))) {
        const int dataBufferOffset = 3 - cd.loopData.loop_end;
#ifdef _LOG
        {
            int x = CubicInterpolator<float>::getIntegerPart(dataBufferOffset + float(cd.curFloatSampleOffset));
            SQINFO("loop, loope end x=%d to %d shift=%d", x - 1, x + 2, dataBufferOffset);
        }
#endif
        float ret = CubicInterpolator<float>::interpolate(cd.loopEndBuffer, float(dataBufferOffset + cd.curFloatSampleOffset));
        cd.advancePointer(lfm);
        return ret * cd.vol;
    } else if (CubicInterpolator<float>::canInterpolate(float(cd.curFloatSampleOffset), cd.frames)) {
        // common case - interp in place
#ifdef _LOG
        {
            int x = CubicInterpolator<float>::getIntegerPart(float(cd.curFloatSampleOffset));
            SQINFO("straight interp, case 1 linear x=%d to %d", x - 1, x + 2);
        }
#endif
        float ret = CubicInterpolator<float>::interpolate(cd.data, float(cd.curFloatSampleOffset));
        cd.advancePointer(lfm);
        return ret * cd.vol;
    } else if (cd.curFloatSampleOffset > (cd.frames - 1)) {
        // if not more data, something is wrong - we ran past end.
        // this can happen with transpose is high..
#ifdef _LOG
        SQINFO("ran past end offset=%f frames=%d", cd.curFloatSampleOffset, cd.frames);
#endif
        return 0;
    } else if (cd.curFloatSampleOffset < 1) {
        // If we are right at the start, we need to use the offset buffer
        // This won't be correct if we are looping
#ifdef _LOG
        {
            int x = CubicInterpolator<float>::getIntegerPart(float(1 + cd.curFloatSampleOffset));
            SQINFO("start: offset buffer interp,  x=%d to %d", x - 1, x + 2);
        }
#endif
        float ret = CubicInterpolator<float>::interpolate(cd.offsetBuffer, float(1 + cd.curFloatSampleOffset));
        cd.advancePointer(lfm);
        return ret * cd.vol;
    } else if (cd.loopActive) {
        assert(false);
    } else if (cd.curFloatSampleOffset >= (cd.frames - 2)) {
        const float subIndex = float(cd.curFloatSampleOffset - (cd.frames - 3));
        assert(subIndex >= 1);

#ifdef _LOG
        {
            // int x = CubicInterpolator<float>::getIntegerPart(float(1 + cd.curFloatSampleOffset));
            int x = CubicInterpolator<float>::getIntegerPart(subIndex);
            SQINFO("end buffer interp,  x=%d to %d", x - 1, x + 2);
        }
#endif
        float ret = CubicInterpolator<float>::interpolate(cd.endBuffer, subIndex);
        cd.advancePointer(lfm);
        return ret * cd.vol;
    }

    if (cd.loopActive) {
        if (cd.loopData.loop_end && cd.curFloatSampleOffset > (cd.loopData.loop_end + 2)) {
            assert(false);
            // const unsigned int loop_length = cd.loopData.loop_end - cd.loopData.loop_start;
            //cd.curFloatSampleOffset -= loop_length;
            //SQINFO("loop wrap, set offset to %d", cd.curFloatSampleOffset);
        }
    }
    assert(false);
    return 0;
}

void Streamer::ChannelData::advancePointer(float lfm) {
#ifdef _LOG
        SQINFO("enter advance, offset=%f, trans=%f", curFloatSampleOffset, transposeMultiplier);
#endif
    curFloatSampleOffset += transposeMultiplier;
    curFloatSampleOffset += lfm;

    // don't let FM push it negative
    curFloatSampleOffset = std::max(0.0, curFloatSampleOffset);

    // original - fails osc3
    //  if (loopActive && (curFloatSampleOffset > loopData.loop_end)) {
    if (loopActive && (curFloatSampleOffset >= (loopData.loop_end + 1))) {
#ifdef _LOG
        SQINFO("in advance loop, offset was %f", curFloatSampleOffset);
        SQINFO("  loop end=%d loop start = %d", loopData.loop_end, loopData.loop_start);
#endif
        // original way - failed osc3 test
        const int loopLength = loopData.loop_end - loopData.loop_start;
        curFloatSampleOffset -= (loopLength + 1);

        // 2: try this instead, but then 4 does not wrap to zero, as it should
        // curFloatSampleOffset -= (loopData.loop_end - loopData.loop_start);
#ifdef _LOG
        SQINFO("after adjust: %f", curFloatSampleOffset);
#endif
        assert(curFloatSampleOffset >= 0);
    }
#ifdef _LOG
    SQINFO("Leaving advancePointer, frames = %d, offset = %f this=%p", frames, curFloatSampleOffset, this);
    SQINFO("tmult = %f, lfm=%f", transposeMultiplier, lfm);
#endif
    if (!loopActive) {
        // if (!CubicInterpolator<float>::canInterpolate(float(curFloatSampleOffset), frames)) {
        if (curFloatSampleOffset > (frames - 1)) {
#ifdef _LOG
            SQINFO("shut off: setting arePlaying = false in advance pointer");
#endif
            arePlaying = false;
        }
    }
}

float Streamer::stepNoTranspose(ChannelData& cd) {
    assert(false);
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

const Streamer::ChannelData& Streamer::_cd(int channel) const {
    assert(channel < 4);
    assert(channel >= 0);
    return channels[channel];
}

bool Streamer::ChannelData::canPlay() const {
    return bool(data && arePlaying);
}

void Streamer::setGain(int channel, float gain) {
    ChannelData& cd = channels[channel];
    cd.gain = gain;
}

void Streamer::setSample(int whichChannel, const float* data, int totalFrames) {
    assert(whichChannel < 4);
    ChannelData& cd = channels[whichChannel];
    if (totalFrames < 4) {
        if (!data) {
            cd.data = data;
            cd.frames = totalFrames;
        }
        assert(totalFrames == 0);
        return;
    }

    cd.data = data;
    cd.frames = totalFrames;
    cd.arePlaying = true;  // this variable doesn't mean much, but???
    cd.curIntegerSampleOffset = 0;
    cd.curFloatSampleOffset = 0;
    cd.vol = 1;
    assert(cd.data);
    assert(cd.frames >= 4);
}

void Streamer::clearSamples() {
    // SQINFO("Streamer::clearSamples()");
    for (int channel = 0; channel < 4; ++channel) {
        clearSamples(channel);
    }
}

void Streamer::clearSamples(int channel) {
    setSample(channel, nullptr, 0);
}

void Streamer::setTranspose(float_4 amount) {
    // SQINFO("Streamer::setTranspose %s", toStr(amount).c_str());

    // TODO: make more efficient!!
    for (int channel = 0; channel < 4; ++channel) {
        ChannelData& cd = channels[channel];
        float xpose = amount[channel];
        float delta = std::abs(xpose - 1);
        bool doTranspose = delta > .0001;  // TODO: is this in tune enough?
        cd.transposeEnabled = doTranspose;
        cd.transposeMultiplier = xpose;
      //  SQINFO("set t=%f on ch%d", cd.transposeMultiplier, channel);
#if 0 // debuging trap
        if (xpose > 2) {
            SQINFO("set t=%f on ch%d", cd.transposeMultiplier, channel);
            assert(false);
        }
#endif
        assert(!std::isinf(cd.transposeMultiplier));
    }
}

void Streamer::ChannelData::_dump() const {
    SQINFO("dumping %p", this);
    SQINFO("vol=%f, te=%d tm=%f g = %f", vol, transposeEnabled, transposeMultiplier, gain);
}

void Streamer::_assertValid() {
    // this is too simplistic now.
#if 0
    for (int channel = 0; channel < 4; ++channel) {
        ChannelData& cd = channels[channel];

        if (cd.transposeEnabled) {
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
#endif
}

bool Streamer::_isTransposed(int channel) const {
    assert(channel < 4);
    const ChannelData& cd = channels[channel];
    return cd.transposeEnabled;
}

float Streamer::_transAmt(int channel) const {
    assert(channel < 4);
    const ChannelData& cd = channels[channel];
    return cd.transposeMultiplier;
}

bool Streamer::blockEnvelopes() const {
    // all are the same, just look at 0
    return channels[0].loopData.loop_mode == SamplerSchema::DiscreteValue::ONE_SHOT;
}

void Streamer::setLoopData(int chan, const CompiledRegion::LoopData& inData) {
    ChannelData& cd = channels[chan];
    assert(0 == cd.curFloatSampleOffset);

    CompiledRegion::LoopData& outData = channels[chan].loopData;
    outData = inData;
    if (inData.oscillator) {
        outData.loop_start = 0;
        outData.loop_end = cd.frames - 1;
        outData.loop_mode = SamplerSchema::DiscreteValue::LOOP_CONTINUOUS;
    }

    bool sqLooped = false;
    switch (outData.loop_mode) {
        case SamplerSchema::DiscreteValue::LOOP_CONTINUOUS:
        case SamplerSchema::DiscreteValue::LOOP_SUSTAIN:
            sqLooped = true;
            break;
        default:
            break;
    }

#if 0
    // assert(chan < 4 && chan >= 0);
    channels[chan].loopData = data;
    if (data.oscillator) {
        data.loop_start = 0;
        data.loop_end = cd.frames - 1;
        data.loop_mode = SamplerSchema::DiscreteValue::LOOP_CONTINUOUS;
    }
#endif

    if (outData.end > 1) {
        channels[chan].frames = std::min(outData.end + 1, channels[chan].frames);
    }
    //channels[chan].loopActive = (data.offset != 0);
    bool valid = false;
    if (outData.loop_start || outData.loop_end) {
        valid = true;
    }
    if (outData.loop_end < outData.loop_start) {
        valid = false;
    }
    if (outData.loop_end >= cd.frames) {
        valid = false;
    }
    if ((outData.loop_end > 0) && (outData.loop_end <= outData.loop_start)) {
        valid = false;
    }
    channels[chan].loopActive = valid && sqLooped;

    // if offset crazy, ignore it
    if (cd.loopData.offset >= cd.frames) {
        cd.loopData.offset = 0;
    }

    // they should have called setSample right before
    assert(0 == cd.curFloatSampleOffset);
    cd.curFloatSampleOffset = outData.offset;

    cd.offsetBuffer[0] = 0;
    cd.endBuffer[3] = 0;
    for (int i = 0; i < 3; ++i) {
        cd.offsetBuffer[i + 1] = cd.data[i];
        cd.endBuffer[i] = cd.data[i + cd.frames - 3];
    }

    if (cd.loopActive) {
        assert(cd.loopData.loop_end >= (cd.loopData.loop_start + 3));
        for (int i = 0; i < 8; ++i) {
            if (i <= 3) {                                           // first four samples are from end of loop
                const int endIndex = i + cd.loopData.loop_end - 3;  // where we get data to move
                cd.loopEndBuffer[i] = cd.data[endIndex];
            } else {
                const int endIndex = i - 2;
                cd.loopEndBuffer[i] = cd.data[endIndex];
            }
        }
    }
#ifdef _LOG
    for (int i = 0; i < 4; ++i) {
        SQINFO("offset buffer[%d]=%f", i, cd.offsetBuffer[i]);
    }
    SQINFO("");
    for (int i = 0; i < 4; ++i) {
        SQINFO("end buffer[%d]=%f", i, cd.endBuffer[i]);
    }
    SQINFO("");
    for (int i = 0; i < 8; ++i) {
        SQINFO("loop_end buffer [%d]=%f", i, cd.loopEndBuffer[i]);
    }
    SQINFO("loopActive = %d, loop end = %d", channels[chan].loopActive, channels[chan].loopData.loop_end);
    SQINFO("offset = %d", channels[chan].loopData.offset);
    SQINFO("------");
#endif
}