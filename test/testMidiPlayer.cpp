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

using TestHostPtr = std::shared_ptr<TestHost>;


// test that APIs can be called
static void test0()
{
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.updateToMetricTime(.01f);
}


/**
 * Makes a one-track song. 
 * Track has one quarter note at t=0, duration = eighth.
 * End event at quarter note end.
 *
 */
MidiSongPtr makeSongOneQ()
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
    pl.updateToMetricTime(time);
    return host;
}


std::shared_ptr<TestHost> makeSongOneQandRun2(float timeBeforeLock, float timeDuringLock, float timeAfterLock)
{
   
    MidiSongPtr song = makeSongOneQ();
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.updateToMetricTime(timeBeforeLock);
    {
        MidiLocker l(song->lock);
        pl.updateToMetricTime(timeBeforeLock + timeDuringLock);
    }

    pl.updateToMetricTime(timeBeforeLock + timeDuringLock + timeAfterLock);
    return host;
}


// just play the first note on
static void test1()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun(2*.24f);

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
    std::shared_ptr<TestHost> host = makeSongOneQandRun2(2*.20f, 2*.01f, 2*.03f);

    assertEQ(host->gateChangeCount, 1);
    assertEQ(host->gateState, true);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvState, 2);
    assertEQ(host->lockConflicts, 1);
}

// play the first note on and off
static void test2()
{
    // this was wall time (1/4 sec)
    //std::shared_ptr<TestHost> host = makeSongOneQandRun(.25f);
    std::shared_ptr<TestHost> host = makeSongOneQandRun(.5f);

    assertEQ(host->lockConflicts, 0);
    assertEQ(host->gateChangeCount, 2);
    assertEQ(host->gateState, false);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvState, 2);
}

// play the first note on and off with lock contention
static void test2L()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun2(2*.20f, 2*.01f, 2*.04f);

    assertEQ(host->lockConflicts, 1);
    assertEQ(host->gateChangeCount, 2);
    assertEQ(host->gateState, false);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvState, 2);
}


// loop around to first note on second time
static void test3()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun(2 * .51f);

    assertEQ(host->gateChangeCount, 3);
    assertEQ(host->gateState, true);
    assertEQ(host->cvChangeCount, 1);       // only changes once because it's one note loop
    assertEQ(host->cvState, 2);
}

// loop around to first note on second time
static void test3L()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun2(2*.4f, 2*.7f, 2*.4f );

    assertGE(host->gateChangeCount, 3);
    assertEQ(host->gateState, true);
    assertGE(host->cvChangeCount, 1);       // only changes once because it's one note loop
    assertEQ(host->cvState, 2);
}


//  double update(int samplesElapsed, bool externalClock);




#if 0
std::shared_ptr<TestHost> makeSongEmptyRun(float time)
{

    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.updateToMetricTime(time);
    return host;
}
#endif


static void testReset()
{
    printf("\n TEST RESET\n");
    // make empty song, player ets.
    // play it a long time
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.updateToMetricTime(100);

    assertEQ(host->gateChangeCount, 0);
    assertEQ(host->gateState, false);
    assertEQ(host->cvChangeCount, 0);

    // Now set new real song
    MidiSongPtr newSong = makeSongOneQ();
    {
        MidiLocker l1(newSong->lock);
        MidiLocker l2(song->lock);
        pl.setSong(newSong);
    }

    // Should play just like it does in test1
    pl.updateToMetricTime(2 * .24f);

    assertEQ(host->lockConflicts, 0);
    assertEQ(host->gateChangeCount, 1);
    assertEQ(host->gateState, true);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvState, 2);
    assertEQ(host->lockConflicts, 0);
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



    testReset();

    assertNoMidi();     // check for leaks
}