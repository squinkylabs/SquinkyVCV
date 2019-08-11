#pragma once

#include "IMidiPlayerHost.h"

#include <algorithm>
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
        notePlayTimer = noteDurationSeconds();
#endif
    }

    void sampleTicksElapsed(int ticks)
    {
#ifdef _DEBUG
        assert(sampleTime > 0);
        if (notePlayTimer > 0) {
            const float elapsedTime = ticks * sampleTime;
            notePlayTimer -= elapsedTime;
            notePlayTimer = std::max(0.f, notePlayTimer);

            if (notePlayTimer == 0) {
                playerHost->setGate(0, false);
            }
        }
#endif
    }

    void setSampleTime(float time)
    {
        sampleTime = time;
    }

    static float noteDurationSeconds()
    {
        return .3f;
    }
private:
    IMidiPlayerHostPtr playerHost;
    float sampleTime = 0;
    float notePlayTimer = 0;

};