#pragma once

#include "IMidiPlayerHost.h"

#include <algorithm>
#include <assert.h>

#if 1
#define _AUDITION
#endif

/**
 * class implements the auditioning of notes from the editor.
 * Note that the threading is a little sloppy - the editor calls
 * auditionNote from the UI thread, everything else is done from the
 * audio thread. Might need some atomic variables.
 */
class MidiAudition : public IMidiPlayerAuditionHost
{
public:
    
    MidiAudition(IMidiPlayerHostPtr h) : playerHost(h)
    {
    }

    /**
     * This method called from the UI thread. 
     */
    void auditionNote(float pitch) override
    {
#ifdef _AUDITION   // disable in real seq until done
        //printf("audition note pitch %.2f retig=%d, playing=%d\n", pitch, isRetriggering, isPlaying);
        if (!enabled) {
            return;
        }

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
                // printf("audition note retrigger set timer sec to %f\n", timerSeconds);
                playerHost->setGate(0, false);
            } 
        }
        // printf("leaving audition,  retig=%d, playing=%d\n", isRetriggering, isPlaying);
#endif
    }

    void sampleTicksElapsed(int ticks)
    {
        if (!enabled) {
            return;
        }
#ifdef _AUDITION
        assert(sampleTime > 0);
        assert(sampleTime < .01);
        if (timerSeconds > 0) {
          
            const float elapsedTime = ticks * sampleTime;
           // printf("counting down timer= %f ticks=%d, st=%f elpased=%f\n", timerSeconds, ticks, sampleTime, elapsedTime); fflush(stdout);
            timerSeconds -= elapsedTime;
            timerSeconds = std::max(0.f, timerSeconds);

            if (timerSeconds == 0) {
               // printf("firing\n");  fflush(stdout);

                if (isRetriggering) {
                    //turn the note back on
                    isRetriggering = false;
                    playerHost->setCV(0, pitchToPlayAfterRetrigger);
                    playerHost->setGate(0, true);

                    // but turn it off after play duration
                    timerSeconds = noteDurationSeconds();
                    isPlaying = true;
                   // printf("audition note retrigger end set timer sec to %f\n", timerSeconds);

                } else {
                    //printf("clearing\n");  fflush(stdout);
                    // timer is just timing down for note
                    playerHost->setGate(0, false);
                }
            }
        }
#endif
    }

    void enable(bool b)
    {
        if (b != enabled) {
            enabled = b;

            // if we just disabled, then shut off audition
            if (!enabled) {
                playerHost->setGate(0, false);
            }
            // any time we change from running to not, clear state.
            isPlaying = false;
            isRetriggering = false;
            timerSeconds = 0;
        }
    }

    void setSampleTime(float time)
    {
        // printf("set sample time %f\n", time);
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
    float timerSeconds = 0;
    bool isRetriggering = false;
    bool isPlaying = false;
    float pitchToPlayAfterRetrigger = 0;
    bool enabled = false;
};