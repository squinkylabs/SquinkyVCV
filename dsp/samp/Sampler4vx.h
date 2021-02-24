#pragma once

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

#define _USEADSR
class Sampler4vx {
public:
    Sampler4vx();
    void note_on(int channel, int midiPitch, int midiVelocity, float sampleRate);
#ifndef _USEADSR
    void note_off(int channel);
#endif

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
#ifdef _USEADSR
    ADSRSampler adsr;
    float_4 R = float_4(.001f);
#endif

    Divider divn;           // used to reduce the polling frequency for remaining samples
    void step_n();
    float sampleTime_ = 0;
    float_4 shutOffNow_ = {0};
    float_4 releaseTime_ = {0};

 //   void tryInit();
};