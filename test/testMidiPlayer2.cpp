
#include "IMidiPlayerHost.h"
#include "MidiLock.h"
#include "MidiPlayer2.h"
#include "MidiSong.h"
#include "MidiVoice.h"
#include "MidiVoiceAssigner.h"

#include "asserts.h"
#include <memory>
#include <vector>


/**
 * mock host to spy on the voices.
 */
class TestHost2 : public IMidiPlayerHost
{
public:

    void setGate(int voice, bool g) override
    {
        assert(voice >= 0 && voice < 16);
        bool bs = gateState[voice];
        bool chg = (bs != g);
        if (g != gateState[voice]) {
            ++gateChangeCount;
            gateState[voice] = g;
        }
    }
    void setCV(int voice, float cv) override
    {
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

    int cvChangeCount = 0;
    int gateChangeCount = 0;

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

using TestHost2Ptr = std::shared_ptr<TestHost2>;

static void test0()
{
    MidiVoice mv;
    MidiVoiceAssigner va(&mv, 1);
}

//************************** MidiVoice tests *********************************************

void initVoices(MidiVoice* voices, int numVoices, IMidiPlayerHost* host)
{
    for (int i = 0; i < numVoices; ++i) {
        voices[i].setHost(host);
        voices[i].setIndex(i);
    }
}

static void testMidiVoiceDefaultState()
{
    MidiVoice mv;
    assert(mv.state() == MidiVoice::State::Idle);
}

static void assertAllButZeroAreInit(TestHost2 *th)
{
    for (int i = 1; i < 16; ++i) {
        assertLT(th->cvValue[i], -10);
        assert(!th->gateState[i]);
    }
}

static void testMidiVoicePlayNote()
{
    TestHost2 th;
    MidiVoice mv;
    mv.setHost(&th);
    mv.playNote(3.f, 0, 1.f);      // pitch 3, dur 1

    assertAllButZeroAreInit(&th);
    assert(mv.state() == MidiVoice::State::Playing);
    assert(th.cvChangeCount == 1);
    assert(th.gateChangeCount == 1);
    assert(th.cvValue[0] == 3.f);
    assert(th.gateState[0] == true);
}


static void testMidiVoicePlayNoteVoice2()
{
    TestHost2 th;
    IMidiPlayerHost* host = &th;
    MidiVoice mv[2];

    initVoices(mv, 2, host);
   
    mv[1].playNote(3.f, 0, 1.f);      // pitch 3, dur 1

    assert(mv[1].state() == MidiVoice::State::Playing);
    assert(th.cvChangeCount == 1);
    assert(th.gateChangeCount == 1);
    assert(th.cvValue[1] == 3.f);
    assert(th.gateState[1] == true);
}

static void testMidiVoicePlayNoteOnAndOff()
{
    TestHost2 th;
    IMidiPlayerHost* host = &th;
    MidiVoice mv;
    initVoices(&mv, 1, host);

    mv.playNote(3.f, 0, 1.f);      // pitch 3, dur 1
    mv.updateToMetricTime(2);   // after note is over

    assertAllButZeroAreInit(&th);
    assert(th.cvValue[0] == 3.f);
    assert(th.gateState[0] == false);
    assert(th.cvChangeCount == 1);
    assert(th.gateChangeCount == 2);
    assert(mv.state() == MidiVoice::State::Idle);
}

static void testMidiVoiceRetrigger()
{
    TestHost2 th;
    IMidiPlayerHost* host = &th;
    MidiVoice mv;
    initVoices(&mv, 1, host);

    mv.playNote(3.f, 0, 1.f);      // pitch 3, dur 1
    assert(th.gateState[0] == true);

    mv.updateToMetricTime(1.0); // note just finished
    assert(th.gateState[0] == false);

    mv.playNote(4.f, 1, 1.f);      // pitch 4, dur 1
    assert(mv.state() == MidiVoice::State::ReTriggering);
    assert(th.gateState[0] == false);
    assert(th.cvValue[0] == 3.f);
    assert(th.gateChangeCount == 2);
    assertAllButZeroAreInit(&th);
}

static void testMidiVoiceRetrigger2()
{
    TestHost2 th;
    IMidiPlayerHost* host = &th;
    MidiVoice mv;
    initVoices(&mv, 1, host);
    mv.setSampleCountForRetrigger(100);

    mv.playNote(3.f, 0, 1.f);      // pitch 3, dur 1
    assert(th.gateState[0] == true);

    mv.updateToMetricTime(1.0); // note just finished
    assert(th.gateState[0] == false);

    mv.playNote(4.f, 1, 1.f);      // pitch 4, dur 1
    assert(mv.state() == MidiVoice::State::ReTriggering);
    assert(th.gateState[0] == false);
    assert(th.cvValue[0] == 3.f);
    assert(th.gateChangeCount == 2);

    mv.updateSampleCount(99);
    assert(th.gateState[0] == false);
    assert(th.gateChangeCount == 2);

    mv.updateSampleCount(1);
    assert(th.gateState[0] == true);
    assert(th.gateChangeCount == 3);
    assert(th.cvValue[0] == 4.f);

    assertAllButZeroAreInit(&th);
}

//************************** MidiVoiceAssigner tests **********************************

static void basicTestOfVoiceAssigner()
{
    MidiVoice vx;
    MidiVoiceAssigner va(&vx, 1);
    auto p = va.getNext(0);
    assert(p);
    assert(p == &vx);
}

static void testVoiceAssign2Notes()
{
    TestHost2 th;
    IMidiPlayerHost* host = &th;
    MidiVoice mv[2];
    initVoices(mv, 2, host);

    MidiVoiceAssigner va(mv, 2);

    auto p = va.getNext(0);
    assert(p);
    assert(p == mv);
    p->playNote(3, 0, 1);

    // first is still playing, so have to get second
    p = va.getNext(0);
    assert(p);
    assert(p == mv+1);
}

static void testVoiceReAssign()
{
    MidiVoice vx;
    MidiVoiceAssigner va(&vx, 1);
    auto p = va.getNext(0);
    assert(p);
    assert(p == &vx);

    p = va.getNext(0);
    assert(p);
    assert(p == &vx);
}

static void testVoiceAssignReUse()
{
    MidiVoice vx[4];
    MidiVoiceAssigner va(vx, 4);
    TestHost2 th;
    va.setNumVoices(4);
    initVoices(vx, 4, &th);


    const float pitch1 = 0;
    const float pitch2 = 1;

    auto p = va.getNext(pitch1);
    assert(p);
    assert(p == vx);
    p->playNote(pitch1, 0, 10);         // play long note to this voice
    assert(p->state() == MidiVoice::State::Playing);

    p = va.getNext(pitch2);
    assert(p);
    assert(p == vx+1);
    p->playNote(pitch2, 0, 10);         // play long note to this voice
    assert(p->state() == MidiVoice::State::Playing);

    //now terminate the notes
    vx[0].updateToMetricTime(20);
    assert(vx[0].state() == MidiVoice::State::Idle);
    vx[1].updateToMetricTime(20);
    assert(vx[1].state() == MidiVoice::State::Idle);

    // now re-allocate pitch 2
    p = va.getNext(pitch2);
    assert(p);
    assert(p == vx+1);
}

static void testVoiceAssingOverflow()
{

    MidiVoice vx[4];
    MidiVoiceAssigner va(vx, 4);
    TestHost2 th;
    va.setNumVoices(4);
    initVoices(vx, 4, &th);


    const float pitch1 = 0;
    const float pitch2 = 1;
    const float pitch3 = 2;
    const float pitch4 = 3;
    const float pitch5 = 4;

    auto p = va.getNext(pitch1);
    assert(p);
    assert(p == vx);
    p->playNote(pitch1, 0, 10);         // play long note to this voice
    assert(p->state() == MidiVoice::State::Playing);

    p = va.getNext(pitch2);
    assert(p);
    assert(p == vx + 1);
    p->playNote(pitch2, 0, 10);         // play long note to this voice
    assert(p->state() == MidiVoice::State::Playing);

    p = va.getNext(pitch3);
    assert(p);
    assert(p == vx + 2);
    p->playNote(pitch3, 0, 10);         // play long note to this voice
    assert(p->state() == MidiVoice::State::Playing);

    p = va.getNext(pitch4);
    assert(p);
    assert(p == vx + 3);
    p->playNote(pitch4, 0, 10);         // play long note to this voice
    assert(p->state() == MidiVoice::State::Playing);

    p = va.getNext(pitch5);
    assert(p);
    assert(p == vx + 0);
    p->playNote(pitch5, 0, 10);         // play long note to this voice
    assert(p->state() == MidiVoice::State::Playing);

  
}


//********************* test helper functions ************************************************

extern MidiSongPtr makeSongOneQ();

static std::shared_ptr<TestHost2> makeSongOneQandRun(float time)
{
    MidiSongPtr song = makeSongOneQ();
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiPlayer2 pl(host, song);
    pl.updateToMetricTime(time);
    return host;
}


static std::shared_ptr<TestHost2> makeSongOneQandRun2(float timeBeforeLock, float timeDuringLock, float timeAfterLock)
{
    MidiSongPtr song = makeSongOneQ();
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiPlayer2 pl(host, song);
    pl.updateToMetricTime(timeBeforeLock);
    {
        MidiLocker l(song->lock);
        pl.updateToMetricTime(timeBeforeLock + timeDuringLock);
    }

    pl.updateToMetricTime(timeBeforeLock + timeDuringLock + timeAfterLock);
    return host;
}

//***************************** MidiPlayer2 ****************************************
// test that APIs can be called
static void testMidiPlayer0()
{
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiPlayer2 pl(host, song);
    pl.updateToMetricTime(.01f);
}

// just play the first note on
// fka test1
static void testMidiPlayerOneNoteOn()
{
    printf("one note\n");
    std::shared_ptr<TestHost2> host = makeSongOneQandRun(2 * .24f);

    assertAllButZeroAreInit(host.get());
    assertEQ(host->lockConflicts, 0);
    assertEQ(host->gateChangeCount, 1);
    assertEQ(host->gateState[0], true);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvValue[0], 2);
    assertEQ(host->lockConflicts, 0);
}

// same as test1, but with a lock contention
static void testMidiPlayerOneNoteOnWithLockContention()
{
    std::shared_ptr<TestHost2> host = makeSongOneQandRun2(2 * .20f, 2 * .01f, 2 * .03f);

    assertAllButZeroAreInit(host.get());
    assertEQ(host->gateChangeCount, 1);
    assertEQ(host->gateState[0], true);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvValue[0], 2);
    assertEQ(host->lockConflicts, 1);
}

// play the first note on and off
// test2
static void testMidiPlayerOneNote()
{
    // this was wall time (1/4 sec)
    //std::shared_ptr<TestHost> host = makeSongOneQandRun(.25f);
    std::shared_ptr<TestHost2> host = makeSongOneQandRun(.5f);

    assertAllButZeroAreInit(host.get());
    assertEQ(host->lockConflicts, 0);
    assertEQ(host->gateChangeCount, 2);
    assertEQ(host->gateState[0], false);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvValue[0], 2);
}

// play the first note on and off with lock contention
static void testMidiPlayerOneNoteLockContention()
{
    std::shared_ptr<TestHost2> host = makeSongOneQandRun2(2 * .20f, 2 * .01f, 2 * .04f);

    assertAllButZeroAreInit(host.get());
    assertEQ(host->lockConflicts, 1);
    assertEQ(host->gateChangeCount, 2);
    assertEQ(host->gateState[0], false);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvValue[0], 2);
}

// loop around to first note on second time
static void testMidiPlayerOneNoteLoop()
{
    std::shared_ptr<TestHost2> host = makeSongOneQandRun(2 * .51f);

    assertAllButZeroAreInit(host.get());
    assertEQ(host->gateChangeCount, 3);
    assertEQ(host->gateState[0], true);
    assertEQ(host->cvChangeCount, 1);       // only changes once because it's one note loop
    assertEQ(host->cvValue[0], 2);
}

// loop around to first note on second time
static void testMidiPlayerOneNoteLoopLockContention()
{
    std::shared_ptr<TestHost2> host = makeSongOneQandRun2(2 * .4f, 2 * .7f, 2 * .4f);

    assertAllButZeroAreInit(host.get());
    assertGE(host->gateChangeCount, 3);
    assertEQ(host->gateState[0], true);
    assertGE(host->cvChangeCount, 1);       // only changes once because it's one note loop
    assertEQ(host->cvValue[0], 2);
}

static void testMidiPlayerReset()
{
    // make empty song, player ets.
    // play it a long time
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::empty, 0);
    std::shared_ptr<TestHost2> host = std::make_shared<TestHost2>();
    MidiPlayer2 pl(host, song);
    pl.updateToMetricTime(100);


    assertAllButZeroAreInit(host.get());
    assertEQ(host->gateChangeCount, 0);
    assertEQ(host->gateState[0], false);
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


    assertAllButZeroAreInit(host.get());
    assertEQ(host->lockConflicts, 0);
    assertEQ(host->gateChangeCount, 1);
    assertEQ(host->gateState[0], true);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvValue[0], 2);
    assertEQ(host->lockConflicts, 0);
}



//*******************************tests of MidiPlayer2 **************************************
void testMidiPlayer2()
{
    test0();
    testMidiVoiceDefaultState();
    testMidiVoicePlayNote();
    testMidiVoicePlayNoteVoice2();
    testMidiVoicePlayNoteOnAndOff();
    testMidiVoiceRetrigger();
    testMidiVoiceRetrigger2();

    basicTestOfVoiceAssigner();
    testVoiceAssign2Notes();
    testVoiceReAssign();
    testVoiceAssignReUse();
    testVoiceAssingOverflow();

    testMidiPlayer0();
    testMidiPlayerOneNoteOn();
    testMidiPlayerOneNoteOnWithLockContention();
    testMidiPlayerOneNote();
    testMidiPlayerOneNoteLockContention();
    testMidiPlayerOneNoteLoop();
    testMidiPlayerOneNoteLoopLockContention();
    testMidiPlayerReset();
}
