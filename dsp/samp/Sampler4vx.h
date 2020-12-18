#pragma once

#include "SimdBlocks.h"
#include <memory>

namespace ci {
    class CompiledInstrument;
    using CompiledInstrumentPtr = std::shared_ptr<CompiledInstrument>;
}

class WaveLoader;
class SInstrument;
using WaveLoaderPtr = std::shared_ptr<WaveLoader>;


#include"Streamer.h"

class Sampler4vx
{
public:
    void note_on(int channel, int midiPitch, int midiVelocity);
    void note_off(int channel);

    void setPatch(ci::CompiledInstrumentPtr inst);
    void setLoader(WaveLoaderPtr loader);

    /**
     * zero to 4
     */
    void setNumVoices(int voices);
    float_4 step();
private:
    ci::CompiledInstrumentPtr patch;
    WaveLoaderPtr waves;
    Streamer player;

    void tryInit();
};