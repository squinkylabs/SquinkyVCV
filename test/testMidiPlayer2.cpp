
#include "IMidiPlayerHost.h"
#include "MidiPlayer2.h"
#include "MidiVoice.h"
#include "MidiVoiceAssigner.h"

#include "asserts.h"
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

void testMidiPlayer2()
{
    test0();
    testMidiVoiceDefaultState();
    testMidiVoicePlayNote();

    basicTestOfVoiceAssigner();
}