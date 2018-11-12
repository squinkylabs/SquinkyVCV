#include "MidiEvent.h"
#include "MidiPlayer.h"
#include "MidiSong.h"
#include "MidiTrack.h"

#include "asserts.h"


class TestHost : public MidiPlayer::IPlayerHost
{
public:
    void setGate(bool g) override
    {
        ++gateCount;
        gateState = g;
    }
    void setCV(float cv) override
    {
        ++cvCount;
        cvState = cv;
    }

    int gateCount = 0;
    bool gateState = false;
    int cvCount = 0;
    float cvState = 0;
};


static void test0()
{
    MidiSongPtr song = MidiSong::makeTest1();
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.timeElapsed(.01f);
}


/**
 * Makes a one-track song. 
 * Track has one quarter note at t=0, duration = eigth.
 * End event at quater note end.
 *
 * noteOnTime = 0 * .5;
 * noteOffTime = .5 * .5;
 */
static MidiSongPtr makeSongOneQ()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    song->createTrack(0);
    MidiTrackPtr track = song->getTrack(0);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 0;
    note->duration = .5;
    note->pitch = 2.f;
    track->insertEvent(note);
    track->insertEnd(1);

    return song;
}


std::shared_ptr<TestHost> makeSongOneQandRun(float time)
{
    MidiSongPtr song = makeSongOneQ();
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.timeElapsed(time);
    return host;
}


// just play the first note on
static void test1()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun(.24f);

    assertEQ(host->gateCount, 1);
    assertEQ(host->gateState, true);
    assertEQ(host->cvCount, 1);
    assertEQ(host->cvState, 2);
}

// play the first note on and off
static void test2()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun(.25f);

    assertEQ(host->gateCount, 2);
    assertEQ(host->gateState, false);
    assertEQ(host->cvCount, 1);
    assertEQ(host->cvState, 2);
}

// loop around to first note on second time
static void test3()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun(.51f);

    assertEQ(host->gateCount, 3);
    assertEQ(host->gateState, true);
    assertEQ(host->cvCount, 2);
    assertEQ(host->cvState, 2);
}

void testMidiPlayer()
{
    test0();
    test1();
    test2();
    test3();
}