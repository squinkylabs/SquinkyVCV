
#include "IMidiPlayerHost.h"
#include "MidiPlayer2.h"
#include "MidiVoice.h"
#include "MidiVoiceAssigner.h"

#include "asserts.h"
#include <vector>


/**
 * mock host to spy on the voices.
 */
class TestHost : public IMidiPlayerHost
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

using TestHostPtr = std::shared_ptr<TestHost>;

static void test0()
{
    MidiPlayer2 mp;
    MidiVoice mv;
    MidiVoiceAssigner va(&mv, 1);
    (void) mp;
}

static void basicTestOfMidiVoice()
{
    MidiVoice mv;
    assert(!mv.isPlaying());
}

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
    basicTestOfMidiVoice();
    basicTestOfVoiceAssigner();
}