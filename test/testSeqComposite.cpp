
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

/**
 * @param clockDiv - 4 for quarter, etc..
 */
std::shared_ptr<Sq> make(SeqClock::ClockRate rate,
    int numVoices, 
    MidiTrack::TestContent testContent)
{
    assert(numVoices > 0 && numVoices <= 16);

    auto song = MidiSong::makeTest(testContent, 0);
    std::shared_ptr<Sq> ret = std::make_shared<Sq>(song);

    ret->params[Sq::NUM_VOICES_PARAM].value = float(numVoices - 1);
    ret->params[Sq::CLOCK_INPUT_PARAM].value = float(rate);        
    ret->inputs[Sq::CLOCK_INPUT].value = 0;        // clock low
    ret->toggleRunStop();                          // start it

    return ret;
}

// makes a seq composite set of for external 8th note clock
// playing 1q song

std::shared_ptr<Sq> makeWith8Clock(bool noteAtTimeZero = false)
{   //need 8 clock
    MidiTrack::TestContent content = noteAtTimeZero ? 
        MidiTrack::TestContent::eightQNotes :
        MidiTrack::TestContent::oneQ1;
    return make(SeqClock::ClockRate::Div2, 16, content);
}


static void testBasicGates()
{
    // Test song is one quater note at time 1
    // eight note clocks
    std::shared_ptr<Sq> s = makeWith8Clock();                          // start it

    stepN(*s, 16);

    assertEQ(s->outputs[Sq::GATE_OUTPUT].channels, 16);
    assertAllGatesLow(*s);

    auto pos = s->getPlayPosition();
    assertLT(pos, 0);

    // Now give first clock - advance to time zero.
    // There is no note at time zero
    genOneClock(*s);
    pos = s->getPlayPosition();
    assertEQ(pos, 0);
    assertAllGatesLow(*s);

    // next clock will take us to the first eighth note, where there is no note
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

static void testStopGatesLow()
{
    std::shared_ptr<Sq> s = makeWith8Clock();

    // step for a while, but with no clock
    stepN(*s, 16);

    assertEQ(s->outputs[Sq::GATE_OUTPUT].channels, 16);
    assertAllGatesLow(*s);
    auto pos = s->getPlayPosition();
    assertLT(pos, 0);


    // now give first clock to time zero
    genOneClock(*s);
    pos = s->getPlayPosition();
    assertEQ(pos, 0);
    assertAllGatesLow(*s);


    // now clock to first eigth
    genOneClock(*s);
    pos = s->getPlayPosition();
    assertEQ(pos, .5);
    assertAllGatesLow(*s);

    // first real Q note, gate high
    genOneClock(*s);
    pos = s->getPlayPosition();
    assertEQ(pos, 1);
    assertGT(s->outputs[Sq::GATE_OUTPUT].voltages[0], 5);

    s->toggleRunStop();     // STOP
    stepN(*s, 16);
    assertAllGatesLow(*s);
}


class TestClocker
{
public:
    TestClocker(std::shared_ptr<Sq> seq) : s(seq)
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
    std::shared_ptr<Sq> s;

    void oneMoreSample()
    {
        sampleCount += 1;       
        if (sampleCount >= samplesPerClock) {
            s->inputs[s->CLOCK_INPUT].value = 10;
            stepN(*s, 16);   // let it get noticed
            s->inputs[s->CLOCK_INPUT].value = 0;
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

static void testRetrigger(bool exactDuration)
{
    // 1/16
    std::shared_ptr<Sq> s = make(SeqClock::ClockRate::Div4, 1,
        exactDuration ? MidiTrack::TestContent::FourTouchingQuarters :
        MidiTrack::TestContent::FourAlmostTouchingQuarters
    );
    TestClocker clock(s);
    

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


static void testResetGatesLow()
{

    std::shared_ptr<Sq> s = makeWith8Clock(true);                          // start it

    stepN(*s, 16);

    auto pos = s->getPlayPosition();
    assertLT(pos, 0);

    // first clock to get to time t=0
    genOneClock(*s);
    pos = s->getPlayPosition();
    assertEQ(pos, 0);

    // now give first advance past zero clock 
    genOneClock(*s);
    pos = s->getPlayPosition();
    assertEQ(pos, .5);
    assertAllGatesLow(*s);

    // first real Q note, gate high
    genOneClock(*s);
    pos = s->getPlayPosition();
    assertEQ(pos, 1);
    assertGT(s->outputs[Sq::GATE_OUTPUT].voltages[0], 5);

    s->toggleRunStop();     // STOP
    stepN(*s, 16);
    assertAllGatesLow(*s);

    // after reset the gates should be low
    s->inputs[Sq::RESET_INPUT].value = 10;
    stepN(*s, 16);
    assertAllGatesLow(*s);

}



void sendClockAndStep(Sq& sq, float clockValue)
{
    sq.inputs[Sq::CLOCK_INPUT].value = clockValue;

    // now step a bit so that we see clock 
    stepN(sq, 4);
}

static void testLoopingQ()
{
    // 1/8 note clock 4 q
    auto song = MidiSong::makeTest(MidiTrack::TestContent::FourTouchingQuartersOct, 0);
    std::shared_ptr<Sq> seq = std::make_shared<Sq>(song);

    seq->params[Sq::NUM_VOICES_PARAM].value = 0;
    seq->params[Sq::CLOCK_INPUT_PARAM].value = float(SeqClock::ClockRate::Div2);
    seq->inputs[Sq::CLOCK_INPUT].value = 0;        // clock low

    // just pause for a bit.
    // we are now "running", but no clocks
    stepN(*seq, 1000);

    assertAllGatesLow(*seq);

    // now start and clock
    seq->inputs[Sq::RUN_STOP_PARAM].value = 10;
    seq->inputs[Sq::CLOCK_INPUT].value = 10;
    assertAllGatesLow(*seq);

    assertEQ(seq->outputs[Sq::CV_OUTPUT].voltages[0], 0);       // no pitch until start

    // now step a bit so that we see clock
    stepN(*seq, 4);

    // should be playing the first note
    assertGT(seq->outputs[Sq::GATE_OUTPUT].voltages[0], 5);
    assertEQ(seq->getPlayPosition(), 0);
    assertEQ(seq->outputs[Sq::CV_OUTPUT].voltages[0], 3);

    // send the clock low
    sendClockAndStep(*seq, 0);

    /* What we need to do (at least on the real clocks) is:
     * send the clock, note that gate is now low, and cv is same
     * wait a bit, note gate goes high and cv advances
     */

    // We are now just started, on the first tick, playing 3V

    assertEQ(seq->getPlayPosition(), 0);
    assertEQ(seq->outputs[Sq::CV_OUTPUT].voltages[0], 3);
    assertGT(seq->outputs[Sq::GATE_OUTPUT].voltages[0], 5);

    // now send clock another clock
    // expect no change, other than advancing an eight note since notes a quarters
    genOneClock(*seq);
    assertEQ(seq->getPlayPosition(), .5);
    assertEQ(seq->outputs[Sq::CV_OUTPUT].voltages[0], 3);
    assertGT(seq->outputs[Sq::GATE_OUTPUT].voltages[0], 5);


    float expectedPos = .5;
    float expectedCV = 3;
    // now, all notes after this will be "the same"
    for (int i = 0; i < 20; ++i) {
        assertEQ(seq->inputs[Sq::CLOCK_INPUT].value, 0);        // precondition for loop, clock low
        /* Clock high and step.
         * This will send the player into re-trigger, since notes touch.
         * In re-trigger we hold the prev CV and force the gate low
         */
        sendClockAndStep(*seq, 10);
        assertEQ(seq->inputs[Sq::CLOCK_INPUT].value, 10);
        expectedPos += .5f;

        // loop around one bar
        bool didLoop = false;
        if (expectedPos > 3.75) {
            expectedPos -= 4;
            didLoop = true;
        }

        assertEQ(seq->getPlayPosition(), expectedPos);
        assertEQ(seq->outputs[Sq::CV_OUTPUT].voltages[0], expectedCV);
        assertLT(seq->outputs[Sq::GATE_OUTPUT].voltages[0], 5);

        // now we are at the onset of a new note, but still re-triggering.
        // wait a bit and should see next note
        stepN(*seq, 50);
        expectedCV += 1;
        if (didLoop) {
            expectedCV -= 4;
        }
        assertEQ(seq->getPlayPosition(), expectedPos);
        assertEQ(seq->outputs[Sq::CV_OUTPUT].voltages[0], expectedCV);
        assertGT(seq->outputs[Sq::GATE_OUTPUT].voltages[0], 5);

        // send clock low
        sendClockAndStep(*seq, 0);

        // next clock will just get us to middle of same note
        genOneClock(*seq);
        expectedPos += .5f;
        assertEQ(seq->getPlayPosition(), expectedPos);
        assertEQ(seq->outputs[Sq::CV_OUTPUT].voltages[0], expectedCV);
        assertGT(seq->outputs[Sq::GATE_OUTPUT].voltages[0], 5);
    }
}

void testSeqComposite()
{
    testBasicGates();
    testStopGatesLow();

    testRetrigger(true);
    testRetrigger(false);

    testResetGatesLow();
    testLoopingQ();
}