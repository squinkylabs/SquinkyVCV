
#include "Seq.h"
#include "TestComposite.h"

#include "asserts.h"

using Sq = Seq<TestComposite>;

static void stepN(Sq& sq, int numTimes)
{
    for (int i = 0; i < numTimes; ++i) {
        sq.step();
    }
}

static void genOneClock(Sq& sq)
{
    sq.inputs[Sq::CLOCK_INPUT].value = 10;
    stepN(sq, 16);
    sq.inputs[Sq::CLOCK_INPUT].value = 0;
    stepN(sq, 16);
}

static void assertAllGatesLow(Sq& sq)
{
    for (int i = 0; i < sq.outputs[Sq::GATE_OUTPUT].channels; ++i) {
        assertEQ(sq.outputs[Sq::GATE_OUTPUT].voltages[i], 0);
    }
}

// makes a seq composite set of for external 8th note clock
// playing 1q song
std::shared_ptr<Sq> makeWith8Clock()
{
    auto song = MidiSong::makeTest(MidiTrack::TestContent::oneQ1, 0);
    std::shared_ptr<Sq> ret = std::make_shared<Sq>(song);

    ret->params[Sq::NUM_VOICES_PARAM].value = 16 - 1;
    ret->params[Sq::CLOCK_INPUT].value = 4;        // 1/8th notes
    ret->inputs[Sq::CLOCK_INPUT].value = 0;        // clock low
    ret->toggleRunStop();                          // start it

    return ret;
}

static void testBasicGates()
{
    std::shared_ptr<Sq> s = makeWith8Clock();                          // start it

    stepN(*s, 16);

    assertEQ(s->outputs[Sq::GATE_OUTPUT].channels, 16);
    assertAllGatesLow(*s);

    auto pos = s->getPlayPosition();
    assertEQ(pos, 0);

    // now give first clock 
    genOneClock(*s);
    pos = s->getPlayPosition();
    assertEQ(pos, .5);
    assertAllGatesLow(*s);

    // first real Q note, gate high
    genOneClock(*s);
    pos = s->getPlayPosition();
    assertEQ(pos, 1);
    assertGT(s->outputs[Sq::GATE_OUTPUT].voltages[0], 5);

    // third real 8th note
    genOneClock(*s);
    pos = s->getPlayPosition();
    assertEQ(pos, 1.5);
    assertGT(s->outputs[Sq::GATE_OUTPUT].voltages[0], 5);
   
    genOneClock(*s);
    pos = s->getPlayPosition();
    assertEQ(pos, 2);
    assertAllGatesLow(*s);
}

void testSeqComposite()
{
    testBasicGates();
}