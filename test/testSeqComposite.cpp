
#include "MidiSong.h"
#include "MidiSong4.h"
#include "Seq.h"
#include "Seq4.h"
#include "TestComposite.h"

#include "asserts.h"

using Sq2 = Seq<TestComposite>;
using Sq4 = Seq4<TestComposite>;

template <class TSeq>
static void stepN(TSeq& sq, int numTimes)
{
    for (int i = 0; i < numTimes; ++i) {
        sq.step();
    }
}

template <class TSeq>
static void genOneClock(TSeq& sq)
{
    sq.inputs[TSeq::CLOCK_INPUT].setVoltage(10, 0);
    stepN(sq, 16);
    sq.inputs[TSeq::CLOCK_INPUT].setVoltage(0, 0);
    stepN(sq, 16);
}

template <class TSeq>
static void assertAllGatesLow(TSeq& sq)
{
    for (int i = 0; i < sq.outputs[TSeq::GATE_OUTPUT].channels; ++i) {
        assertEQ(sq.outputs[TSeq::GATE_OUTPUT].voltages[i], 0);
    }
}

// TODO: move to a general UTIL
template <typename T>
static void initParams(T * composite)
{
    auto icomp = composite->getDescription();
    for (int i = 0; i < icomp->getNumParams(); ++i) {
        auto param = icomp->getParam(i);
        composite->params[i].value = param.def;
    }
}

/**
 * @param clockDiv - 4 for quarter, etc..
 */
template <class TSeq, class TSong>
std::shared_ptr<TSeq> make(SeqClock::ClockRate rate,
    int numVoices, 
    MidiTrack::TestContent testContent,
    bool toggleStart)
{
    assert(numVoices > 0 && numVoices <= 16);

    std::shared_ptr <TSong> song = TSong::makeTest(testContent, 0);
    auto ret = std::make_shared<TSeq>(song);

    // we SHOULD init the params properly for all the tests,
    // but not all work. this is a start.
    if (!toggleStart) {
        initParams(ret.get());
    }


    const float f = ret->params[TSeq::RUNNING_PARAM].value;

    ret->params[TSeq::NUM_VOICES_PARAM].value = float(numVoices - 1);
    ret->params[TSeq::CLOCK_INPUT_PARAM].value = float(rate);
    ret->inputs[TSeq::CLOCK_INPUT].setVoltage(0, 0);        // clock low
    if (toggleStart) {
        ret->toggleRunStop();                          // start it
    }

    return ret;
}

// makes a seq composite set of for external 8th note clock
// playing 1q song

template <class TSeq, class TSong>
std::shared_ptr<TSeq> makeWith8Clock(bool noteAtTimeZero = false)
{   //need 8 clock
    MidiTrack::TestContent content = noteAtTimeZero ? 
        MidiTrack::TestContent::eightQNotes :
        MidiTrack::TestContent::oneQ1;
    return make<TSeq, TSong>(SeqClock::ClockRate::Div2, 16, content, true);
}

template <class TSeq, bool hasPlayPosition>
static void testBasicGatesSub(std::shared_ptr<TSeq> s)
{
    float f = s->params[TSeq::RUNNING_PARAM].value;
    stepN(*s, 16);
    f = s->params[TSeq::RUNNING_PARAM].value;

    assertEQ(s->outputs[TSeq::GATE_OUTPUT].channels, 16);
    assertAllGatesLow(*s);
#if hasPlayPosition
    auto pos = s->getPlayPosition();
    assertLT(pos, 0);
#endif
    f = s->params[TSeq::RUNNING_PARAM].value;

    // Now give first clock - advance to time zero.
    // There is no note at time zero
    genOneClock(*s);
#if hasPlayPosition
    pos = s->getPlayPosition();
    assertEQ(pos, 0);
#endif
    assertAllGatesLow(*s);
    f = s->params[TSeq::RUNNING_PARAM].value;

    // next clock will take us to the first eighth note, where there is no note
    genOneClock(*s);
#if hasPlayPosition
    pos = s->getPlayPosition();
    assertEQ(pos, .5);
#endif
    assertAllGatesLow(*s);
    f = s->params[TSeq::RUNNING_PARAM].value;

    // first real Q note, gate high
    genOneClock(*s);
#if hasPlayPosition
    pos = s->getPlayPosition();
    assertEQ(pos, 1);
#endif
    assertGT(s->outputs[TSeq::GATE_OUTPUT].voltages[0], 5);

    // third real 8th note
    genOneClock(*s);
#if hasPlayPosition
    pos = s->getPlayPosition();
    assertEQ(pos, 1.5);
#endif
    assertGT(s->outputs[TSeq::GATE_OUTPUT].voltages[0], 5);
   
    genOneClock(*s);
#if hasPlayPosition
    pos = s->getPlayPosition();
    assertEQ(pos, 2);
#endif
    assertAllGatesLow(*s);
}

template <class TSeq, class TSong, bool hasPlayPosition>
static void testBasicGates()
{
    std::shared_ptr<TSeq> s = makeWith8Clock<TSeq, TSong>();                          // start it
    const float f = s->params[TSeq::RUNNING_PARAM].value;
    testBasicGatesSub<TSeq, hasPlayPosition>(s);
}

template <class TSeq, class TSong, bool hasPlayPosition>
static void testStopGatesLow()
{
    float pos = 0;
    std::shared_ptr<TSeq> s = makeWith8Clock<TSeq, TSong>();

    // step for a while, but with no clock
    stepN(*s, 16);

    assertEQ(s->outputs[TSeq::GATE_OUTPUT].channels, 16);
    assertAllGatesLow(*s);

#if hasPlayPosition
    pos = s->getPlayPosition();
    assertLT(pos, 0);
#endif


    // now give first clock to time zero
    genOneClock(*s);

#if hasPlayPosition
    pos = s->getPlayPosition();
    assertEQ(pos, 0);
#endif

    assertAllGatesLow(*s);


    // now clock to first eigth
    genOneClock(*s);
#if hasPlayPosition
    pos = s->getPlayPosition();
    assertEQ(pos, .5);
#endif
    assertAllGatesLow(*s);

    // first real Q note, gate high
    genOneClock(*s);
#if hasPlayPosition
    pos = s->getPlayPosition();
    assertEQ(pos, 1);
#endif
    assertGT(s->outputs[TSeq::GATE_OUTPUT].voltages[0], 5);

    s->toggleRunStop();     // STOP
    stepN(*s, 16);
    assertAllGatesLow(*s);
}

template <class TSeq>
class TestClocker
{
public:
    TestClocker(std::shared_ptr<TSeq> seq) : s(seq)
    {
            // want samples Per Clokm
            //120 bpm, one q = .5
            float secondsPerClock = .125f;        // 1/6th at 120bpm
            //float secondsPerSample = 1.f / 44100.f;
            float samplesPerSecond = 44100.f;
            samplesPerClock = secondsPerClock * samplesPerSecond;
    }

    /**
     * clocks the composite until we generate a clock low to high
     */
    void advanceToClock()
    {
        for (bool done = false; !done; ) {
            if (clockOccured) {
                clockOccured = false;
                done = true;
            } else {
                oneMoreSample();
            }
        }
    }

    /**
     * @returns the number of samples the output was low
     */
    int advanceSampleCountUntilGateHigh()
    {
        int count = 0;
        for (bool done = false; !done; ) {
            ++count;
            s->step();
            if (s->outputs[s->GATE_OUTPUT].voltages[0] > 5.f) {
                done = true;
            }
        }
        return count;
    }

private:
    std::shared_ptr<TSeq> s;

    void oneMoreSample()
    {
        sampleCount += 1;       
        if (sampleCount >= samplesPerClock) {
            s->inputs[s->CLOCK_INPUT].setVoltage(10, 0);
            stepN(*s, 16);   // let it get noticed
            s->inputs[s->CLOCK_INPUT].setVoltage(0, 0);
            stepN(*s, 16);   // let it get noticed
            sampleCount -= samplesPerClock;
            clockOccured = true;

            totalSamples += 32;
        }
        s->step();
        totalSamples += 1;
    }

    float samplesPerClock;
    float sampleCount = 0;
    bool clockOccured = false;
    float totalSamples = 0;
};

template <class TSeq, class TSong>
static void testRetrigger(bool exactDuration)
{
    // 1/16
    std::shared_ptr<TSeq> s = make<TSeq, TSong>(SeqClock::ClockRate::Div4, 1,
        exactDuration ? MidiTrack::TestContent::FourTouchingQuarters :
        MidiTrack::TestContent::FourAlmostTouchingQuarters,
        true
    );
    TestClocker<TSeq> clock(s);
    

    assert(s->outputs[s->GATE_OUTPUT].voltages[0] < 5);

    // first tick will play the note at time zero. don't need a clock
    // wait - that's wrong. we (now) do require a clock
    stepN(*s, 16);   // let it get noticed
    assert(s->outputs[s->GATE_OUTPUT].voltages[0] < 5);

    // now, three more 1/6 note clocks - should all be in the first note - no triggers
    for (int i = 0; i < 4; ++i) {
        clock.advanceToClock();
        assert(s->outputs[s->GATE_OUTPUT].voltages[0] > 5);
    }

    // next clock should cause a re-trigger, so get should be low after that
    clock.advanceToClock();
    assert(s->outputs[s->GATE_OUTPUT].voltages[0] < 5);

    // now we are done with first note, in the re-trigger period before second note
    int x = clock.advanceSampleCountUntilGateHigh();

    // there is a lot of slop in the test in in the div4 step in Seq
    // Really as long it takes a couple clocks to get back we know re-trigger is working
    assert(x <= 44 && x > 8);       
}


template <class TSeq, class TSong>
static void testResetGatesLow()
{
    // make a seq with note at time zero, eight eighth notes
    std::shared_ptr<TSeq> s = makeWith8Clock<TSeq, TSong>(true);                          // start it

    stepN(*s, 16);

    float pos = 0;
#if hasPlayPosition
    pos = s->getPlayPosition();
    assertLT(pos, 0);
#endif

    // first clock to get to time t=0
    // this will play first note
    genOneClock(*s);
#if hasPlayPosition
    pos = s->getPlayPosition();
    assertEQ(pos, 0);
#endif
    assertGT(s->outputs[TSeq::GATE_OUTPUT].voltages[0], 5);   // now note playing

    s->toggleRunStop();     // STOP
    stepN(*s, 16);
    assertAllGatesLow(*s);

    // after reset the gates should be low
    s->inputs[TSeq::RESET_INPUT].setVoltage(10, 0);
    stepN(*s, 16);
    assertAllGatesLow(*s);
}


template <class TSeq>
void sendClockAndStep(TSeq& sq, float clockValue)
{
    sq.inputs[TSeq::CLOCK_INPUT].setVoltage(clockValue, 0);

    // now step a bit so that we see clock 
    stepN(sq, 4);
}

extern float lastTime;
template <class TSeq, class TSong, bool hasPlayPosition>
static void testLoopingQ()
{
    lastTime = 0;
    // 1/8 note clock 4 q
    auto song = TSong::makeTest(MidiTrack::TestContent::FourTouchingQuartersOct, 0);
    std::shared_ptr<TSeq> seq = std::make_shared<TSeq>(song);
    seq->params[TSeq::NUM_VOICES_PARAM].value = 0;
    seq->params[TSeq::CLOCK_INPUT_PARAM].value = float(SeqClock::ClockRate::Div2);
    seq->inputs[TSeq::CLOCK_INPUT].setVoltage(0);        // clock low

    // just pause for a bit.
    // we are now "running", but no clocks
    stepN(*seq, 1000);

    assertAllGatesLow(*seq);

    // now start and clock
    // How does this acutally start?
 //   seq->inputs[Sq::RUN_STOP_PARAM].value = 10;
    seq->inputs[TSeq::RUN_INPUT].setVoltage(10, 0);
    seq->inputs[TSeq::CLOCK_INPUT].setVoltage(10, 0);
    assertAllGatesLow(*seq);

    assertEQ(seq->outputs[TSeq::CV_OUTPUT].voltages[0], 0);       // no pitch until start

    // now step a bit so that we see clock
    stepN(*seq, 4);

    // should be playing the first note
    assertGT(seq->outputs[TSeq::GATE_OUTPUT].voltages[0], 5);
#if hasPlayPosition
    assertEQ(seq->getPlayPosition(), 0);
#endif
    assertEQ(seq->outputs[TSeq::CV_OUTPUT].voltages[0], 3);

    // send the clock low
    sendClockAndStep(*seq, 0);

    /* What we need to do (at least on the real clocks) is:
     * send the clock, note that gate is now low, and cv is same
     * wait a bit, note gate goes high and cv advances
     */

    // We are now just started, on the first tick, playing 3V

#if hasPlayPosition
    assertEQ(seq->getPlayPosition(), 0);
#endif
    assertEQ(seq->outputs[TSeq::CV_OUTPUT].voltages[0], 3);
    assertGT(seq->outputs[TSeq::GATE_OUTPUT].voltages[0], 5);

    // now send clock another clock
    // expect no change, other than advancing an eight note since notes a quarters
    genOneClock(*seq);
#if hasPlayPosition
    assertEQ(seq->getPlayPosition(), .5);
#endif
    assertEQ(seq->outputs[TSeq::CV_OUTPUT].voltages[0], 3);
    assertGT(seq->outputs[TSeq::GATE_OUTPUT].voltages[0], 5);


    float expectedPos = .5;
    float expectedCV = 3;
    // now, all notes after this will be "the same"
    for (int i = 0; i < 20; ++i) {
        assertEQ(seq->inputs[TSeq::CLOCK_INPUT].getVoltage(0), 0);        // precondition for loop, clock low
        /* Clock high and step.
         * This will send the player into re-trigger, since notes touch.
         * In re-trigger we hold the prev CV and force the gate low
         */
        sendClockAndStep(*seq, 10);
        assertEQ(seq->inputs[TSeq::CLOCK_INPUT].getVoltage(0), 10);
        expectedPos += .5f;

        // loop around one bar
        bool didLoop = false;
        if (expectedPos > 3.75) {
            expectedPos -= 4;
            didLoop = true;
        }
#if hasPlayPosition
        assertEQ(seq->getPlayPosition(), expectedPos);
#endif
        assertEQ(seq->outputs[TSeq::CV_OUTPUT].voltages[0], expectedCV);
        assertLT(seq->outputs[TSeq::GATE_OUTPUT].voltages[0], 5);

        // now we are at the onset of a new note, but still re-triggering.
        // wait a bit and should see next note
        stepN(*seq, 50);
        expectedCV += 1;
        if (didLoop) {
            expectedCV -= 4;
        }
#if hasPlayPosition
        assertEQ(seq->getPlayPosition(), expectedPos);
#endif
        assertEQ(seq->outputs[TSeq::CV_OUTPUT].voltages[0], expectedCV);
        assertGT(seq->outputs[TSeq::GATE_OUTPUT].voltages[0], 5);

        // send clock low
        sendClockAndStep(*seq, 0);

        // next clock will just get us to middle of same note
        genOneClock(*seq);
        expectedPos += .5f;
#if hasPlayPosition
        assertEQ(seq->getPlayPosition(), expectedPos);
#endif
        assertEQ(seq->outputs[TSeq::CV_OUTPUT].voltages[0], expectedCV);
        assertGT(seq->outputs[TSeq::GATE_OUTPUT].voltages[0], 5);
    }
}

template <class TSong>
extern std::shared_ptr<TSong> makeSongOneQ(float noteTime, float endTime);

template <class TSeq, class TSong>
static void testSubrangeLoop()
{
    // make a song with one note in the second bar.
    // loop the second bar
    MidiSongPtr song = makeSongOneQ<MidiSong>(4, 100);
    SubrangeLoop lp(true, 4, 8);
    song->setSubrangeLoop(lp);
    std::shared_ptr<TSeq> seq = std::make_shared<TSeq>(song);
    seq->params[TSeq::NUM_VOICES_PARAM].value = 0;
    seq->params[TSeq::CLOCK_INPUT_PARAM].value = float(SeqClock::ClockRate::Div2);
    seq->inputs[TSeq::CLOCK_INPUT].setVoltage(0, 0);        // clock low

    // now step a bit so that we see low inputs (run and clock)
    stepN(*seq, 4);

    assertAllGatesLow(*seq);
    assertEQ(seq->outputs[TSeq::CV_OUTPUT].voltages[0], 0);       // no pitch until start

    // now start and clock
    seq->inputs[TSeq::RUN_INPUT].setVoltage(10, 0);
    seq->inputs[TSeq::CLOCK_INPUT].setVoltage(10, 0);

    // now step a bit so that we see clock
    stepN(*seq, 4);

    assertGT(seq->outputs[TSeq::GATE_OUTPUT].voltages[0], 5);

    // should be back around to another loops starting in the first bar.
    assertEQ(seq->getPlayPosition(), 4);

}

template <class TSeq>
static void step(std::shared_ptr<TSeq> seq)
{
    for (int i = 0; i < 16; ++i) {
        seq->step();
    }
}

template <class TSeq>
static void testStepRecord()
{
    // DrumTrigger<TestComposite>;
    std::shared_ptr<TSeq> seq = makeWith8Clock<TSeq, MidiSong>();

    seq->params[TSeq::STEP_RECORD_PARAM].value = 1;
    seq->inputs[TSeq::GATE_INPUT].channels = 1;
    seq->inputs[TSeq::GATE_INPUT].voltages[0] = 10;
    seq->inputs[TSeq::CV_INPUT].voltages[0] = 2;
    step(seq);

    RecordInputData buffer;
    bool b = seq->poll(&buffer);
    assert(b);
    assert(buffer.type == RecordInputData::Type::noteOn);
    assert(buffer.pitch == 2);

    b = seq->poll(&buffer);
    assert(!b);
    step(seq);
    b = seq->poll(&buffer);
    assert(!b);

    seq->inputs[TSeq::GATE_INPUT].voltages[0] = 0;
    step(seq);

    b = seq->poll(&buffer);
    assert(b);
    assert(buffer.type == RecordInputData::Type::allNotesOff);
}

template<class TSeq, class TSong>
static void testGateReset()
{
    printf("TEST GATE RESET\n");
    std::shared_ptr<TSeq> seq = makeWith8Clock<TSeq, TSong>();
    setGates(seq, true);

    // set it, so will clear gates
    seq->inputs[TSeq::RESET_INPUT].voltages[0] = 10.f;
    step(seq);
    assertGates(seq, false);
}

template <class TSeq>
void setGates(std::shared_ptr<TSeq>, bool);


template <class TSeq>
void assertGates(std::shared_ptr<TSeq>, bool);

static void setGates(std::shared_ptr<Sq2> seq, bool state)
{
   for (int i = 0; i < 15; ++i) {
        seq->outputs[Sq2::GATE_OUTPUT].voltages[i] = state ? 10.f : 0.f;
   }
}

static void assertGates(std::shared_ptr<Sq2> seq, bool state)
{
    for (int i = 0; i < 15; ++i) {
        float g = seq->outputs[Sq2::GATE_OUTPUT].voltages[i];
        float expected = state ? 10.f : 0.f;
        assertEQ(g, expected);
    }
}

static void setGates(std::shared_ptr<Sq4> seq, bool state)
{
    for (int output = 0; output < 4; ++output) {
        for (int i = 0; i < 15; ++i) {
            seq->outputs[Sq4::GATE_OUTPUT + output].voltages[i] = state ? 10.f : 0.f;
        }
    }
}

static void assertGates(std::shared_ptr<Sq4> seq, bool state)
{
    for (int output = 0; output < 4; ++output) {
        for (int i = 0; i < 15; ++i) {
          
            float g = seq->outputs[Sq4::GATE_OUTPUT + output].voltages[i];

            float expected = state ? 10.f : 0.f;
            assertEQ(g, expected);
        }
    }
}

template<class TSeq, class TSong, bool hasPlayPosition>
static void testCommon()
{
    testBasicGates<TSeq, TSong, hasPlayPosition>();
    testStopGatesLow<TSeq, TSong, hasPlayPosition>();
    testRetrigger<TSeq, TSong>(true);
    testRetrigger<TSeq, TSong>(false);
    testResetGatesLow<TSeq, TSong>();
    testLoopingQ<TSeq, TSong, hasPlayPosition>();
    testGateReset<TSeq, TSong>();
}

void testSeqComposite()
{
    testCommon<Sq2, MidiSong, true>();
    testCommon<Sq4, MidiSong4, false>();

    testSubrangeLoop<Sq2, MidiSong>();
    testStepRecord<Sq2>();
}