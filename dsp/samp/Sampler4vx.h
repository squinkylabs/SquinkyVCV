#pragma once

#include "SimdBlocks.h"
#include <memory>

class WaveLoader;
class SInstrument;
using WaveLoaderPtr = std::shared_ptr<WaveLoader>;
using SInstrumentPtr = std::shared_ptr<SInstrument>;

class Sampler4vx
{
public:
    void note_on(int channel, int midiPitch, int midiVelocity);
    void note_off(int channel);

    void setPatch(SInstrumentPtr inst) {
        patch = inst;
    }
    void setLoader(WaveLoaderPtr loader) {
        waves = loader;
    }

    /**
     * zero to 4
     */
    void setNumVoices(int voices);
    float_4 step();
private:
    SInstrumentPtr patch;
    WaveLoaderPtr waves;
};