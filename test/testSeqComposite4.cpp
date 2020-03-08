


#include "Seq4.h"
#include "asserts.h"

extern MidiSong4Ptr makeTestSong4(int trackNum);


using Sq4 = Seq4<TestComposite>;
using Sq4Ptr = std::shared_ptr<Sq4>;

// TODO: move to a general UTIL
template <typename T>
static void initParams(T* composite)
{
    auto icomp = composite->getDescription();
    for (int i = 0; i < icomp->getNumParams(); ++i) {
        auto param = icomp->getParam(i);
        composite->params[i].value = param.def;
    }
}

/**
 * adapted from one in testSeqComposite
 * @param clockDiv - 4 for quarter, etc..
 */

std::shared_ptr<Sq4> make(SeqClock::ClockRate rate,
    int numVoices,
    bool toggleStart,
    int trackNum)
{
    assert(numVoices > 0 && numVoices <= 16);

    std::shared_ptr <MidiSong4> song = makeTestSong4(trackNum);

    auto ret = std::make_shared<Sq4>(song);

    // we SHOULD init the params properly for all the tests,
    // but not all work. this is a start.
    if (!toggleStart) {
        initParams(ret.get());
    }


    const float f = ret->params[Sq4::RUNNING_PARAM].value;

    ret->params[Sq4::NUM_VOICES_PARAM].value = float(numVoices - 1);
    ret->params[Sq4::CLOCK_INPUT_PARAM].value = float(rate);
    ret->inputs[Sq4::CLOCK_INPUT].setVoltage(0, 0);        // clock low
    if (toggleStart) {
        ret->toggleRunStop();                          // start it
    }

    return ret;
}


/*

here's the test that passes for track player


   // play 3/4 of first
    play(pl, 3, quantizationInterval);
    assertEQ(pl.getSection(), 1);

    printf("test will pause and seek\n");
    pl.setRunningStatus(false);         // pause
    pl.setNextSectionRequest(4);        // goto last section

    printf("test will resume\n");
    lastTime = -100;

    pl.setRunningStatus(true);          // resume
    play(pl, .1, quantizationInterval); // play a tinny bit
    assertEQ(pl.getSection(), 4);       // should be playing requested section
    play(pl, .1, quantizationInterval); // play a tinny bit
    assertEQ(pl.getSection(), 4);       // should be playing requested section

    play(pl, 7.9, quantizationInterval); // play most (this section 2 bars)
    assertEQ(pl.getSection(), 4);       // should be playing requested section

    play(pl, 8.1, quantizationInterval); // play most (this section 2 bars)
    assertEQ(pl.getSection(), 1);       // should be playing requested section


*/


static void stepN(Sq4Ptr sq, int numTimes)
{
    for (int i = 0; i < numTimes; ++i) {
        sq->step();
    }
}


static void genOneClock(Sq4Ptr sq)
{
    sq->inputs[Sq4::CLOCK_INPUT].setVoltage(10, 0);
    stepN(sq, 16);
    sq->inputs[Sq4::CLOCK_INPUT].setVoltage(0, 0);
    stepN(sq, 16);
}

static void play(std::shared_ptr<Sq4> comp, SeqClock::ClockRate rate, float quarterNotes)
{
    assert(rate == SeqClock::ClockRate::Div64);
    const int clocks = int(64.f * quarterNotes);
    for (int i = 0; i < clocks; ++i) {
        genOneClock(comp);
    }
}

// test seq is 1,2,2,2 bars
// very basic test to make sure our scffolding works
static void test0()
{
    const int tkNum = 0;
    const auto rate = SeqClock::ClockRate::Div64;
    Sq4Ptr comp = make(rate, 4, true, tkNum);
    MidiTrackPlayerPtr pl = comp->getTrackPlayer(tkNum);

    // play to third quarter note
    play(comp, rate, 3.f);
    assertEQ(pl->getSection(), 1);      // first section is 1

    // play start of next section
    play(comp, rate, 1.1f);
    assertEQ(pl->getSection(), 2); 
}


void testSeqComposite4()
{

    test0();
}