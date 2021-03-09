
#pragma once

#include "SimdBlocks.h"
#include "SqLog.h"

class EdgeCatcher {
    public:
    EdgeCatcher(float thresh, const char* l) : threshold (thresh), label(l) {}
    bool sample(float s) {
        if (isFirstValue) {
            lastValue = s;
            isFirstValue = false;
            return false;
        }
        bool r = false;
        float delta = std::abs(lastValue - s);
        if (delta > threshold) {
            if (!havePrinted) {
                SQINFO("");
                SQINFO("jump detectd at %s size %f", label.c_str(), delta);
                SQINFO("value is %f, was %f", s, lastValue);
                havePrinted = true;
                r = true;
            }
           
        }
        lastValue = s;
        return r;
    }
private:
    float threshold;
    float lastValue;
    bool isFirstValue = true;
    std::string label;
    bool havePrinted = false;

};

/**
 * This is a four channel streamer.
 * Streamer is the thing that plays out a block of samples, possibly at an
 * altered rate.
 */
class Streamer {
public:
   // Streamer();
    void setSample(int chan, float* data, int frames);
  //  void setTranspose(int chan, bool doTranspose, float amount);

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
        float* data = nullptr;
        int frames = 0;

        float vol = 1;  // this will go away when we have envelopes

        int curIntegerSampleOffset = 0;
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

        void _dump() const;
    };
    ChannelData channels[4];

    float stepNoTranspose(ChannelData&);
    float stepTranspose(ChannelData&, const float lfm);

};
