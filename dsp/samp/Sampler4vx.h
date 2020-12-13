#pragma once

#include "SimdBlocks.h"

class Sampler4vx
{
public:
    void note_on(int channel, int midiPitch, int midiVoltage);
    void note_off(int channel);

    /**
     * zero to 4
     */
    void setNumVoices(int voices);
    float_4 step();
};