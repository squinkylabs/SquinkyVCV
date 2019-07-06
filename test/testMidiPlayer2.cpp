
#include "IMidiPlayerHost.h"
#include "MidiPlayer2.h"
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
    MidiPlayer2 mp;
    MidiVoice mv;
    MidiVoiceAssigner va(&mv, 1);
    (void) mp;
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

static void testMidiVoicePlayNote()
{
    TestHost2 th;
    MidiVoice mv;
    mv.setHost(&th);
    mv.playNote(3.f, 1.f);      // pitch 3, dur 1

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
   
    mv[1].playNote(3.f, 1.f);      // pitch 3, dur 1

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

    mv.playNote(3.f, 1.f);      // pitch 3, dur 1
    mv.updateToMetricTime(2);   // after note is over

   
    assert(th.cvValue[0] == 3.f);
    assert(th.gateState[0] == false);
    assert(th.cvChangeCount == 1);
    assert(th.gateChangeCount == 2);
    assert(mv.state() == MidiVoice::State::Idle);

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
    p->playNote(3, 1);

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

//*********************************************************************
void testMidiPlayer2()
{
    test0();
    testMidiVoiceDefaultState();
    testMidiVoicePlayNote();
    testMidiVoicePlayNoteVoice2();
    testMidiVoicePlayNoteOnAndOff();
  
    basicTestOfVoiceAssigner();
    testVoiceAssign2Notes();
    testVoiceReAssign();
}
