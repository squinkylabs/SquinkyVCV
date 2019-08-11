#pragma once

#include "IMidiPlayerHost.h"

#include <assert.h>

class MidiAudition : public IMidiPlayerAuditionHost
{
public:
    

    MidiAudition(IMidiPlayerHostPtr h) : playerHost(h)
    {

    }

    void auditionNote(float pitch) override
    {
#ifdef _DEBUG   // disable in real seq until done
        playerHost->setCV(0, pitch);
        playerHost->setGate(0, true);
#endif
    }

    void sampleTicksElapsed(int ticks)
    {
        assert(sampleTime > 0);
    }

    void setSampleTime(float time)
    {
        sampleTime = time;
    }
    float noteDurationSeconds() const
    {
        return .3f;
    }
private:
    IMidiPlayerHostPtr playerHost;
    float sampleTime = 0;
    float notePlayTimer = 0;

};