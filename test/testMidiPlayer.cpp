#include "MidiEvent.h"
#include "MidiLock.h"
#include "MidiPlayer.h"
#include "MidiSong.h"
#include "MidiTrack.h"

#include "asserts.h"

static void testLock()
{
    MidiLock l;
    assert(!l.locked());
    bool b = l.playerTryLock();
    assert(b);
    assert(l.locked());

    l.playerUnlock();
    assert(!l.locked());
    l.editorLock();
    assert(l.locked());
    
    b = l.playerTryLock();
    assert(!b);
    l.editorUnlock();
    assert(!l.locked());
    b = l.playerTryLock();
    assert(b);
    assert(l.locked());

    l.playerUnlock();
    assert(!l.locked());
}


static void testLock2()
{
    MidiLockPtr l = MidiLock::make();
    assert(!l->locked());
    bool b = l->playerTryLock();
    assert(b);
    assert(l->locked());

    l->playerUnlock();
    assert(!l->locked());

    {
        MidiLocker _(l);
        assert(l->locked());
        b = l->playerTryLock();
        assert(!b);
    }
    assert(!l->locked());
    b = l->playerTryLock();
    assert(b);
    assert(l->locked());

    l->playerUnlock();
    assert(!l->locked());
}

static void testLock3()
{
    MidiLockPtr l = MidiLock::make();
    l->editorLock();
    l->editorLock();
    assert(l->locked());

    l->editorUnlock();
    assert(l->locked());

    l->editorUnlock();
    assert(!l->locked());
}

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
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
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
    MidiLocker l(song->lock);
    song->createTrack(0);
    MidiTrackPtr track = song->getTrack(0);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 0;
    note->duration = .5;
    note->pitchCV = 2.f;
    track->insertEvent(note);
    track->insertEnd(1);

    return song;
}


std::shared_ptr<TestHost> makeSongOneQandRun(float time)
{
    MidiSongPtr song = makeSongOneQ();
    MidiLocker l(song->lock);
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
    testLock();
    testLock2();
    testLock3();
    test0();
    test1();
    test2();
    test3();
    assertNoMidi();     // check for leaks
}