#include "MidiEvent.h"
#include "MidiLock.h"
#include "MidiPlayer.h"
#include "MidiSong.h"
#include "MidiTrack.h"
#include "SeqClock.h"

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

static void testLock4()
{
    MidiLockPtr l = MidiLock::make();
    assert(!l->dataModelDirty());
    l->editorLock();
    l->editorUnlock();
    assert(l->dataModelDirty());
    assert(!l->dataModelDirty());
}

class TestHost : public IPlayerHost
{
public:
    void setGate(bool g) override
    {
        if (g != gateState) {
            ++gateChangeCount;
            gateState = g;
        }
    }
    void setCV(float cv) override
    {
        if (cv != cvState) {
            ++cvChangeCount;
            cvState = cv;
        }
    }
    void onLockFailed() override
    {
        ++lockConflicts;
    }

    int cvChangeCount = 0;
    int gateChangeCount = 0;
    bool gateState = false;

    float cvState = -100;

    int lockConflicts = 0;
};


// test that APIs can be called
static void test0()
{
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.timeElapsed(.01f);
}


/**
 * Makes a one-track song. 
 * Track has one quarter note at t=0, duration = eighth.
 * End event at quarter note end.
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
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.timeElapsed(time);
    return host;
}

std::shared_ptr<TestHost> makeSongOneQandRun2(float timeBeforeLock, float timeDuringLock, float timeAfterLock)
{
   
    MidiSongPtr song = makeSongOneQ();
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.timeElapsed(timeBeforeLock);
    {
        MidiLocker l(song->lock);
        pl.timeElapsed(timeDuringLock);
    }

    pl.timeElapsed(timeAfterLock);
    return host;
}

// just play the first note on
static void test1()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun(.24f);

    assertEQ(host->lockConflicts, 0);
    assertEQ(host->gateChangeCount, 1);
    assertEQ(host->gateState, true);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvState, 2);
    assertEQ(host->lockConflicts, 0);
}

// same as test1, but with a lock contention
static void test1L()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun2(.20f, .01f, .03f);

    assertEQ(host->gateChangeCount, 1);
    assertEQ(host->gateState, true);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvState, 2);
    assertEQ(host->lockConflicts, 1);
}



// play the first note on and off
static void test2()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun(.25f);

    assertEQ(host->lockConflicts, 0);
    assertEQ(host->gateChangeCount, 2);
    assertEQ(host->gateState, false);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvState, 2);
}

// play the first note on and off
static void test2L()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun2(.20f, .01f, .04f);

    assertEQ(host->lockConflicts, 1);
    assertEQ(host->gateChangeCount, 2);
    assertEQ(host->gateState, false);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvState, 2);
}
// loop around to first note on second time
static void test3()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun(.51f);

    assertEQ(host->gateChangeCount, 3);
    assertEQ(host->gateState, true);
    assertEQ(host->cvChangeCount, 1);       // only changes once because it's one note loop
    assertEQ(host->cvState, 2);
}

// loop around to first note on second time
static void test3L()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun2(.4f, .7f, .4f );

    assertGE(host->gateChangeCount, 3);
    assertEQ(host->gateState, true);
    assertGE(host->cvChangeCount, 1);       // only changes once because it's one note loop
    assertEQ(host->cvState, 2);
}

/*
  double update(int samplesElapsed, bool externalClock);
    void setup(int inputSetting, float tempoSetting);
    */

// test internal clock
static void testClock0()
{
    const int sampleRateI = 44100;
    const float sampleRate = float(sampleRateI);
    const float sampleTime = 1.f / sampleRate;
    SeqClock ck;
    ck.setup(0, 120, sampleTime);       // internal clock

    // now clock by one second
    double elapsed = ck.update(sampleRateI, false);

    // quarter note = half second at 120,
    // so one second = 2q
    assertEQ(elapsed, 2.0);

    elapsed = ck.update(sampleRateI, false);
    assertEQ(elapsed, 4.0);

    ck.reset();
    ck.setup(0, 240, sampleTime);       // internal clock
    elapsed = ck.update(sampleRateI * 10, false);
    assertEQ(elapsed, 40);
}

//  double update(int samplesElapsed, bool externalClock);

static void testClockExt(int rate, double metricTimePerClock)
{
    assertGT(rate, 0);
    assertLE(rate, 5);

    SeqClock ck;
    ck.setup(rate, 120, 100);       // internal clock
    
    // send one clock
    for (int i = 0; i < 10; ++i) {
        double x = ck.update(55, false);        // low clock
        assertEQ(x, 0);
    }

    // count home much metric time comes back
    double x = ck.update(55, true);
    assertEQ(x, metricTimePerClock);
    
}

static void testClock1()
{
    testClockExt(5, 1.0);
    testClockExt(4, 1.0 / 2.0);
    testClockExt(3, 1.0 / 4.0);
    testClockExt(2, 1.0 / 8.0);
    testClockExt(1, 1.0 / 16.0);
}

void testMidiPlayer()
{
    assertNoMidi();
    testLock();
    testLock2();
    testLock3();
    testLock4();

    test0();
    test1();
    test2();
    test3();

    test1L();
    test2L();
    test3L();

    testClock0();
    testClock1();

    assertNoMidi();     // check for leaks
}