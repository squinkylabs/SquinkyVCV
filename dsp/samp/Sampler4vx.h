#pragma once

#include <algorithm>
#include <memory>

#include "ADSRSampler.h"
#include "Divider.h"
#include "SimdBlocks.h"

class CompiledInstrument;
using CompiledInstrumentPtr = std::shared_ptr<CompiledInstrument>;

class WaveLoader;
class SInstrument;
using WaveLoaderPtr = std::shared_ptr<WaveLoader>;

#include "Streamer.h"


// fordebugging
#include <utility>
class Accumulator {
public:
    Accumulator(int divPeriod) : period(divPeriod) {}

    bool go(float x) {
        bool ret = false;
        min = std::min(x, min);
        max = std::max(x, max);
        if (++counter > period) {
            ret = true;
            counter = 0;
        }
        return ret;
    }
    std::pair<float, float> get() {
        auto ret = std::make_pair(min, max);
        min = 10;
        max = -10;
        return ret;
    }
private:
    const int period;
    int counter = 0;
    float min = 10;
    float max = -10;
};

#define _USEADSR
class Sampler4vx {
public:
    Sampler4vx();
    void note_on(int channel, int midiPitch, int midiVelocity, float sampleRate);


    void setPatch(CompiledInstrumentPtr inst);
    void setLoader(WaveLoaderPtr loader);

    /**
     * zero to 4
     */
    void setNumVoices(int voices);
    float_4 step(const float_4& gates, float sampleTime);

    // fixed
    static float_4 _outputGain() {
        return 5;
    }

private:
    CompiledInstrumentPtr patch;
    WaveLoaderPtr waves;
    Streamer player;
    ADSRSampler adsr;

    // Don't remember what this is for
    float_4 R = float_4(.001f);
    Divider divn;           // used to reduce the polling frequency for remaining samples
    void step_n();
    float sampleTime_ = 0;
    float_4 shutOffNow_ = {0};
    float_4 releaseTime_ = {0};

   // Accumulator acc = {100};
};