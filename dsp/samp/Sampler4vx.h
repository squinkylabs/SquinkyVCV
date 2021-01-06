#pragma once

#include <memory>

#include "ADSR4.h"
#include "SimdBlocks.h"

class CompiledInstrument;
using CompiledInstrumentPtr = std::shared_ptr<CompiledInstrument>;

class WaveLoader;
class SInstrument;
using WaveLoaderPtr = std::shared_ptr<WaveLoader>;

#include "Streamer.h"

class Sampler4vx {
public:
    void note_on(int channel, int midiPitch, int midiVelocity);
    void note_off(int channel);

    void setPatch(CompiledInstrumentPtr inst);
    void setLoader(WaveLoaderPtr loader);

    /**
     * zero to 4
     */
    void setNumVoices(int voices);
    float_4 step(const float_4& gates, float sampleTime);

private:
    CompiledInstrumentPtr patch;
    WaveLoaderPtr waves;
    Streamer player;
    ADSR4 adsr;
    float_4 R = float_4(.001f);


    void tryInit();
};