
#include "Streamer.h"

#include <assert.h>
#include <stdio.h>

#include <algorithm>

#include "CubicInterpolator.h"
#include "SqLog.h"

#define _INTERP

float_4 Streamer::step(float_4 fm, bool fmEnabled) {
    float_4 ret;
    // SQINFO("St:Step %d, %s", fmEnabled, toStr(fm).c_str());
    for (int channel = 0; channel < 4; ++channel) {
        ChannelData& cd = channels[channel];

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
    float ret = 0;

    // first try interp in place
    if (CubicInterpolator<float>::canInterpolate(float(cd.curFloatSampleOffset), cd.frames)) {
         ret = CubicInterpolator<float>::interpolate(cd.data, float(cd.curFloatSampleOffset));
    } else if (cd.curFloatSampleOffset < cd.frames) {      
        // OK, can't in place, but we can still do it
        if (cd.curFloatSampleOffset < 1) {
            // interp from sample before start, add a zero
            const float y0 = 0;
            const float y1 = cd.data[0];
            const float y2 = cd.data[1];
            const float y3 = cd.data[2];
            ret = CubicInterpolator<float>::interpolate(float(cd.curFloatSampleOffset), y0, y1, y2, y3);

        } else {
            // for now, we only know three kinds of interp:
            // normal in-place, insert zero at the end, and put two zeros at the end
            //assert(false);
            unsigned int index = CubicInterpolator<float>::getIntegerPart(float(cd.curFloatSampleOffset));
            const float y0 = cd.data[index-1];
            const float y1 = cd.data[index];
            float y2 = 0;
            float y3 = 0;
            if (cd.curFloatSampleOffset + 1 < cd.frames) {
                y2 = cd.data[index + 1];
            }
            ret = CubicInterpolator<float>::interpolate(float(cd.curFloatSampleOffset), y0, y1, y2, y3);
           
        }

    } else {
        cd.arePlaying = false;
        return 0;
    }
    

    // advance the sample offset
    cd.curFloatSampleOffset += cd.transposeMultiplier;
    cd.curFloatSampleOffset += lfm;

    // don't let FM push it negative
    cd.curFloatSampleOffset = std::max(0.0, cd.curFloatSampleOffset);


    if (!CubicInterpolator<float>::canInterpolate(float(cd.curFloatSampleOffset), cd.frames)) {
        cd.arePlaying = false;
    }

    return ret * cd.vol;
}

#if 0
float Streamer::stepTranspose(ChannelData& cd, float lfm) {
    float ret = 0;
    
    // first try to interpolate in place
    if (CubicInterpolator<float>::canInterpolate(float(cd.curFloatSampleOffset), cd.frames)) {
        if (!cd.arePlaying) {
            SQWARN("why are we playing a streamer not started? pl=%d, fo=%f, io=%d", cd.arePlaying, cd.curFloatSampleOffset, cd.curIntegerSampleOffset);
            assert(false);
        }

#ifdef _INTERP
        ret = CubicInterpolator<float>::interpolate(cd.data, float(cd.curFloatSampleOffset));
#else
        a b
            size_t index = cd.curFloatSampleOffset;
        ret = cd.data[index];
#endif
        // advance the sample offset
        cd.curFloatSampleOffset += cd.transposeMultiplier;
        cd.curFloatSampleOffset += lfm;

        // don't let FM push it negative
        cd.curFloatSampleOffset = std::max(0.0, cd.curFloatSampleOffset);
    }
    else {
        assert(false);
    }

    if (!CubicInterpolator<float>::canInterpolate(float(cd.curFloatSampleOffset), cd.frames)) {
        cd.arePlaying = false;
    }

    return ret * cd.vol;
}
#endif

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

bool Streamer::canPlay(int channel) {
    assert(channel < 4);
    const ChannelData& cd = channels[channel];
    return bool(cd.data && cd.arePlaying);
}

void Streamer::setGain(int channel, float gain) {
    ChannelData& cd = channels[channel];
    cd.gain = gain;
}

void Streamer::setSample(int whichChannel, const float* data, int totalFrames) {
    assert(whichChannel < 4);
    ChannelData& cd = channels[whichChannel];

    // temporary validity test
#if 0  // ndef NDEBUG
    // SQINFO("st::setSample(%d) siz=%d", channel, f);
    for (int i = 0; i < totalFrames; ++i) {
        const float x = data[i];
        assert(x <= 1);
        assert(x >= -1);
    }
#endif
    cd.data = data;
    cd.frames = totalFrames;
    cd.arePlaying = true;  // this variable doesn't mean much, but???
    cd.curIntegerSampleOffset = 0;

    // New way - no "off by one" on the sample counts, will do some more work..
    cd.curFloatSampleOffset = 0;
   // old way, add one to make the math easier for interp
   // cd.curFloatSampleOffset = 1;  // start one past, to allow for interpolator padding
    cd.vol = 1;
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
    // TODO: make more efficient!!
    for (int channel = 0; channel < 4; ++channel) {
        ChannelData& cd = channels[channel];
        float xpose = amount[channel];
        float delta = std::abs(xpose - 1);
        bool doTranspose = delta > .0001;  // TODO: is this in tune enough?

#if 0
        if ((doTranspose != cd.transposeEnabled) || (xpose != cd.transposeMultiplier)) {
            SQINFO("");
            SQINFO("pose was %d / %f", cd.transposeEnabled, cd.transposeMultiplier);
            SQINFO("will be %d / %f", doTranspose, xpose);
        }
#endif
        cd.transposeEnabled = doTranspose;
        cd.transposeMultiplier = xpose;

#if 0
        if (myIndex == 0 && channel == 0) {
            SQINFO("Streamer::setTranspose %f, %d", xpose, doTranspose)
        }
#endif
    }
}

#if 0
void Streamer::setTranspose(int channel, bool doTranspose, float amount) {
    SQINFO("streamer trans ch=%d amt=%f\n", channel, amount);
    assert(channel < 4);
    ChannelData& cd = channels[channel];
    cd.transposeEnabled = doTranspose;
    cd.transposeMultiplier = amount;
}
#endif

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


void Streamer::setLoopData(int chan, const CompiledRegion::LoopData& data) {
   // assert(chan < 4 && chan >= 0);
    channels[chan].loopData = data;
    channels[chan].loopActive = (data.offset != 0);
    bool valid = false;
    if (data.offset || data.loop_start || data.loop_end) {
        valid = true;
    }
    if (data.loop_end < data.loop_start) {
        valid = false;
    }
    channels[chan].loopActive = valid;
 }