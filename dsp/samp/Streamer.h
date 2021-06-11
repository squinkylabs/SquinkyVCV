
#pragma once

#include "CompiledRegion.h"
#include "SimdBlocks.h"
#include "SqLog.h"

/**
 * This is a four channel streamer.
 * Streamer is the thing that plays out a block of samples, possibly at an
 * altered rate.
 */
class Streamer {
public:
    Streamer() = default;
    Streamer(const Streamer&) = delete;
    void setSample(int chan, const float* data, int frames);
    void setLoopData(int chan, const CompiledRegion::LoopData& data);


    /**
     * set the instantaneous pitch.
     * @param amount is the ratio, so 1 means no transpose.
     */
    void setTranspose(float_4 amount);
    bool canPlay(int chan);
    void clearSamples();
    void clearSamples(int channel);
    void setGain(int chan, float gain);

    /** here "fm" is the linear fm modulation,
     * not the pitch modulation
     */
    float_4 step(float_4 fm, bool fmEnabled);
    void _assertValid();

    bool _isTransposed(int channel) const;
    float _transAmt(int channel) const;

public:
    class ChannelData {
    public:
        /*
         * buffer that holds the first three samples, in case we are offset.
         * 0, s0, s1, s2
         */
        float offsetBuffer[4] = {0};  

        /*
         *  buffer that hold the last three samples
         *   sn-2, sn-1, sn, 0
         */   
        float endBuffer[4] = {0};          
       // float loopStartBuffer[4] = {0};
        float loopEndBuffer[4] = {0};
        const float* data = nullptr;
        unsigned int frames = 0;

        float vol = 1;  // this will go away when we have envelopes

        unsigned int curIntegerSampleOffset = 0;
        bool arePlaying = false;

        /* If curFloatSampleOffset is a float we build up error and it 
         * makes the pitch off by a few cents. We could use an integer
         * and just use the float for the fraction, but I have a feeling that 
         * just using a double is simpler and faster.
         */
        double curFloatSampleOffset = 0;

        bool transposeEnabled = false;
        float transposeMultiplier = 1;
        float gain = 1;
        CompiledRegion::LoopData loopData;

        /**
         * true if playback needs to look at loop data. 
         */
        bool loopActive = false;

        void _dump() const;
        void advancePointer(float lfm);
    };
    ChannelData channels[4];

    float stepNoTranspose(ChannelData&);
    float stepTranspose(ChannelData&, const float lfm);
};
