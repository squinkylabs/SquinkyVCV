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
        if (seqIsRunning) {
            return;
        }
#ifdef _DEBUG   // disable in real seq until done
        
        if (!isPlaying && !isRetriggering) {
            // starting a new note
            playerHost->setCV(0, pitch);
            playerHost->setGate(0, true);
            timerSeconds = noteDurationSeconds();
            isPlaying = true;
        } else {
            // play when already playing
            // we will re-trigger, or at least change pitch
            pitchToPlayAfterRetrigger = pitch;
            if (!isRetriggering) {
                isRetriggering = true;
                isPlaying = false;
                timerSeconds = retriggerDurationSeconds();      
                playerHost->setGate(0, false);
            }
          
        }
#endif
    }

    void sampleTicksElapsed(int ticks)
    {
        if (seqIsRunning) {
            return;
        }
#ifdef _DEBUG
        assert(sampleTime > 0);
        if (timerSeconds > 0) {
            const float elapsedTime = ticks * sampleTime;
            timerSeconds -= elapsedTime;
            timerSeconds = std::max(0.f, timerSeconds);

            if (timerSeconds == 0) {
                //playerHost->setGate(0, false);
                if (isRetriggering) {
                    isRetriggering = false;
                    playerHost->setCV(0, pitchToPlayAfterRetrigger);
                    playerHost->setGate(0, true);
                } else {
                    // timer is just timing down for note
                    playerHost->setGate(0, false);
                }
            }
        }
#endif
    }

    void setRunningStatus(bool b)
    {
        seqIsRunning = b;
    }

    void setSampleTime(float time)
    {
        sampleTime = time;
    }

    static float noteDurationSeconds()
    {
        return .3f;
    }

    static float retriggerDurationSeconds()
    {
        return .001f;
    }
private:
    IMidiPlayerHostPtr playerHost;
    float sampleTime = 0;
   // float notePlayTimer = 0;
    float timerSeconds = 0;
    bool isRetriggering = false;
    bool isPlaying = false;
    float pitchToPlayAfterRetrigger = 0;
    bool seqIsRunning = true;
};