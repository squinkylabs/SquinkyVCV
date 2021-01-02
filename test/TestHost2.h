#pragma once

#include "IMidiPlayerHost.h"
#include "MidiVoice.h"

/**
 * mock host to spy on the voices.
 */
class TestHost2 : public IMidiPlayerHost4
{
public:
    void assertOneActiveTrack(int index) {
        assertEQ(index, 0);
    }
    void reset()
    {
        cvChangeCount = 0;
        gateChangeCount = 0;
        for (auto it : gateState) {
            it = false;
        }
        for (auto it : cvValue) {
            it = -100;
        }
    }
    void setGate(int track, int voice, bool g) override
    {
#ifdef _MLOG
        printf("test host setGate(%d) -> %d\n", voice, g);
#endif
        assert(track == 0 || !g);       // if we have events on unexpected tracks, they are just gate off from reset
        assert(voice >= 0 && voice < 16);
        bool bs = gateState[voice];
        bool chg = (bs != g);
        if (g != gateState[voice]) {
            ++gateChangeCount;
            gateState[voice] = g;
        }
    }
    void setCV(int track, int voice, float cv) override
    {
        assert(track == 0);
        assert(voice >= 0 && voice < 16);
        if (cv != cvValue[voice]) {
            ++cvChangeCount;
            cvValue[voice] = cv;
        }
    }

    void onLockFailed() override
    {
        ++lockConflicts;
    }

    void resetClock() override
    {
        // won't reset anything, but remember we were called
       ++numClockResets;
    }

    int cvChangeCount = 0;
    int gateChangeCount = 0;
    int numClockResets = 0;

    int lockConflicts = 0;

    std::vector<bool> gateState = {
        false, false, false, false,
        false, false, false, false,
        false, false, false, false,
        false, false, false, false};
    std::vector<float> cvValue = {
        -100,-100,-100,-100,
        -100,-100,-100,-100,
        -100,-100,-100,-100,
        -100,-100,-100,-100};
};
