
#include "OneShot.h"
#include "SeqClock.h"
#include "asserts.h"

static void testOneShotInit()
{
    OneShot o;
    // should always be fired, until triggered
    assert(o.hasFired());
    o.setDelayMs(1);
    assert(o.hasFired());
    o.setSampleTime(1.f / 44100.f);
    assert(o.hasFired());
    o.set();
    assert(!o.hasFired());

}

static void testOneShot2Ms()
{
    OneShot o;
    o.setSampleTime(.001f); // sample rate 1k
    o.setDelayMs(2);        // delay 2ms
    assert(o.hasFired());
    o.set();
    assert(!o.hasFired());

    o.step();               // 1 ms.
    assert(!o.hasFired());

    o.step();               // 2 ms.
    assert(o.hasFired());

    for (int i = 0; i < 10; ++i) {
        o.step();
        assert(o.hasFired());
    }

    o.set();
    assert(!o.hasFired());

    o.step();               // 1 ms.
    assert(!o.hasFired());

    o.step();               // 2 ms.
    assert(o.hasFired());

}

//  SeqClock::update(int samplesElapsed, float externalClock, float runStop, float reset)
// test internal clock
static void testClockInternal0()
{
    const int sampleRateI = 44100;
    const float sampleRate = float(sampleRateI);
    const float sampleTime = 1.f / sampleRate;

    SeqClock ck;
    ck.setup(SeqClock::ClockRate::Internal, 120, sampleTime);       // internal clock

    // now clock by one second
    SeqClock::ClockResults results = ck.update(sampleRateI, 0, true, 0);

    // quarter note = half second at 120,
    // so one second = 2q
    assertEQ(results.totalElapsedTime, 2.0);
    assert(!results.didReset);

    results = ck.update(sampleRateI, 0, true, 0);
    assertEQ(results.totalElapsedTime, 4.0);

    ck.reset(true);
    ck.setup(SeqClock::ClockRate::Internal, 240, sampleTime);       // internal clock
    results = ck.update(sampleRateI * 10, 0, true, 0);
    assertEQ(results.totalElapsedTime, 40);
  //  assertGT(results.totalElapsedTime, 38.9);
 //   assertLE(results.totalElapsedTime, 40);
// with original reset this was 40. new reset clock logic maybe doesn't work right with internal
    
    assert(!results.didReset);
}

static void testClockExt(SeqClock::ClockRate rate, double metricTimePerClock)
{
   // assertGT(rate, 0);
   // assertLE(rate, 5);

    SeqClock ck;
    ck.setup(rate, 120, 100);       // internal clock

    SeqClock::ClockResults results;
    // send one clock
    for (int i = 0; i < 10; ++i) {
        results = ck.update(55, 0, true, 0);        // low clock
        assertEQ(results.totalElapsedTime, 0);
    }

    // count home much metric time comes back
    results = ck.update(55, 10, true, 0);
    assertEQ(results.totalElapsedTime, metricTimePerClock);

}

static void testClockExt1()
{
    testClockExt(SeqClock::ClockRate::Div1, 1.0);
    testClockExt(SeqClock::ClockRate::Div2, 1.0 / 2.0);
    testClockExt(SeqClock::ClockRate::Div4, 1.0 / 4.0);
    testClockExt(SeqClock::ClockRate::Div8, 1.0 / 8.0);
    testClockExt(SeqClock::ClockRate::Div16, 1.0 / 16.0);
}

static void testClockExtEdge()
{
  //  const int rate = 5;
    auto rate = SeqClock::ClockRate::Div1;
    const double metricTimePerClock = 1;
    SeqClock ck;
    SeqClock::ClockResults results;
    ck.setup(rate, 120, 100);       // internal clock

    // send one clock (first low)
    for (int i = 0; i < 10; ++i) {
        results = ck.update(55, 0, true, 0);        // low clock
        assertEQ(results.totalElapsedTime, 0);
    }

    // then high once
    results = ck.update(55, 10, true, 0);
    assertEQ(results.totalElapsedTime, metricTimePerClock);

    // then high some more
    for (int i = 0; i < 10; ++i) {
        results = ck.update(55, 10, true, 0);        // low clock
        assertEQ(results.totalElapsedTime, metricTimePerClock);
    }

     // low more
    for (int i = 0; i < 10; ++i) {
        results = ck.update(55, 0, true, 0);        // low clock
        assertEQ(results.totalElapsedTime, metricTimePerClock);
    }

     // then high some more
    for (int i = 0; i < 10; ++i) {
        results = ck.update(55, 10, true, 0);        // low clock
        assertEQ(results.totalElapsedTime, 2 * metricTimePerClock);
    }
}

static void testClockInternalRunStop()
{
    const int sampleRateI = 44100;
    const float sampleRate = float(sampleRateI);
    const float sampleTime = 1.f / sampleRate;

    SeqClock ck;
    ck.setup(SeqClock::ClockRate::Internal, 120, sampleTime);       // internal clock

    // now clock by one second
    SeqClock::ClockResults results = ck.update(sampleRateI, 0, true, 0);

    // quarter note = half second at 120,
    // so one second = 2q
    assertEQ(results.totalElapsedTime, 2.0);

    // now clock stopped, should not run
    results = ck.update(sampleRateI, 0, 0, 0);
    assertEQ(results.totalElapsedTime, 2.0);

     // now on again, should run
    results = ck.update(sampleRateI, 0, true, 0);
    assertEQ(results.totalElapsedTime, 4.0);
}

static void testClockChangeWhileStopped()
{
    const int sampleRateI = 44100;
    const float sampleRate = float(sampleRateI);
    const float sampleTime = 1.f / sampleRate;

    SeqClock ck;
    ck.setup(SeqClock::ClockRate::Div1, 120, sampleTime);       // external clock

    // call with clock low, running
    SeqClock::ClockResults results = ck.update(sampleRateI, 0, true, 0);
    assertEQ(results.totalElapsedTime, 0);

    // now stop
    results = ck.update(sampleRateI, 0, false, 0);
    assertEQ(results.totalElapsedTime, 0);

    // raise clock while stopped
    for (int i = 0; i < 10; ++i) {
        results = ck.update(sampleRateI, 10, false, 0);
    }
    assertEQ(results.totalElapsedTime, 0);

    // now run. see if we catch the edge
    results = ck.update(sampleRateI, 10, true, 0);
    assertEQ(results.totalElapsedTime, 1);
}

static void testSimpleReset()
{
    const int sampleRateI = 44100;
    const float sampleRate = float(sampleRateI);
    const float sampleTime = 1.f / sampleRate;

    SeqClock ck;
    SeqClock::ClockResults results;
    ck.setup(SeqClock::ClockRate::Internal, 120, sampleTime);       // internal clock

    // one second goes by at 120 -> half note
    results = ck.update(sampleRateI, 0, true, 0);
    assert(!results.didReset);
    assertEQ(results.totalElapsedTime, 2.f);

    // now reset. should send reset, and set us back to zero, but
    // not suppress us
    results = ck.update(sampleRateI, 0, true, 10);
    assert(results.didReset);
    assertEQ(results.totalElapsedTime, 2.f);

    results = ck.update(sampleRateI, 0, true, 10);
    assert(!results.didReset);

    results = ck.update(sampleRateI, 0, true, 10);
    assert(!results.didReset);

    results = ck.update(sampleRateI, 0, true, 0);
    assert(!results.didReset);
}

static void testSimpleResetIgnoreClock()
{
    const int sampleRateI = 44100;
    const float sampleRate = float(sampleRateI);
    const float sampleTime = 1.f / sampleRate;

    SeqClock ck;
    SeqClock::ClockResults results;
    ck.setup(SeqClock::ClockRate::Div16, 120, sampleTime);       // external clock tempo 120

    // run external clock high
    results = ck.update(sampleRateI, 10, true, 0);
    assert(!results.didReset);
    const double t0 = results.totalElapsedTime;

    // clock low and high
    ck.update(sampleRateI, 0, true, 0);
    results = ck.update(sampleRateI, 10, true, 0);
    const double t1 = results.totalElapsedTime;

    assertGT(t1, t0);       // we are clocking now

    // now reset
    results = ck.update(sampleRateI, 10, true, 10);
    assert(results.didReset);
    assertLT(results.totalElapsedTime, 0);      // reset should set clock back to waiting


    results = ck.update(sampleRateI, 0, true, 0);
    assert(!results.didReset);

    //   ClockResults update(int samplesElapsed, float externalClock, bool runStop, float reset);

    // clock should be locked out now
    results = ck.update(1, 10, true, 0);
    assert(!results.didReset);
}


static void testResetIgnoreClock()
{
    const int sampleRateI = 44100;
    const float sampleRate = float(sampleRateI);
    const float sampleTime = 1.f / sampleRate;

    int samplesInOneMs = int(44100.f / 1000.f);

    SeqClock ck;
    SeqClock::ClockResults results;
    ck.setup(SeqClock::ClockRate::Div1, 120, sampleTime);       // external clock = quarter

    // run external clock high
    results = ck.update(sampleRateI, 10, true, 0);
    assert(!results.didReset);
    const double t0 = results.totalElapsedTime;

    // clock low and high
    ck.update(sampleRateI, 0, true, 0);
    results = ck.update(sampleRateI, 10, true, 0);
    const double t1 = results.totalElapsedTime;

    assertClose(t1 - t0, 1, .0001);     // quarter note elapsed from one clock edge

    // now reset
    results = ck.update(1, 10, true, 10);
    assert(results.didReset);
    assertLT(results.totalElapsedTime, 0);      // reset should set clock back to waiting

    //   ClockResults update(int samplesElapsed, float externalClock, bool runStop, float reset);

    int errorMargin = 10;
    // step for a little under one ms

    for (int i = 0; i < (samplesInOneMs - errorMargin); ++i) {
        results = ck.update(1, 0, true, 0);
        assertLT(results.totalElapsedTime, 0);
    }

    // this clock should be ignored
    results = ck.update(1, 10, true, 0);
    assertLT(results.totalElapsedTime, 0);

    // step for a little more with clock low
    for (int i = 0; i < 2 * errorMargin; ++i) {
        results = ck.update(1, 0, true, 0);
        assertLT(results.totalElapsedTime, 0);
    }

    // this clock should NOT be ignored
    results = ck.update(sampleRateI, 10, true, 0);
    assertEQ(results.totalElapsedTime, 0);      // first clock after reset advance to start
   // assertClose(results.totalElapsedTime, 1, .000001);
}

static void testRates()
{
    const int num = int( SeqClock::ClockRate::NUM_CLOCKS);
    auto labels = SeqClock::getClockRates();
    assert(num == labels.size());
    for (std::string label : labels) {
        assert(!label.empty());
    }
}

static void testNoNoteAfterReset()
{
    const int sampleRateI = 44100;
    const float sampleRate = float(sampleRateI);
    const float sampleTime = 1.f / sampleRate;

    SeqClock ck;
    SeqClock::ClockResults results;
    ck.setup(SeqClock::ClockRate::Div1, 120, sampleTime);       // external clock = quarter

    //   ClockResults update(int samplesElapsed, float externalClock, bool runStop, float reset)
    
    
    // clock it a bit

    for (int j = 0; j < 10; ++j) {
         results = ck.update(100, 0, true, 0);
         results = ck.update(100, 10, true, 0);
    }

    // stop it
    results = ck.update(1, 0, false, 0);
    assert(!results.didReset);

    // reset it
    results = ck.update(1, 0, false, 10);
    assert(results.didReset);

    // after reset we should output time before start, so that when we finally start we play the first note
    assertLT(results.totalElapsedTime, 0);

    // while we are stopped, time should still not pass, even if clocked
    for (int i = 0; i < 100; ++i) {
        results = ck.update(100, 0, false, 0);
        assert(!results.didReset);
        assertLT(results.totalElapsedTime, 0);

        results = ck.update(100, 10, false, 0);
        assert(!results.didReset);
        assertLT(results.totalElapsedTime, 0);
    }

    // now let it run (but no clock)
    for (int i = 0; i < 100; ++i) {
        results = ck.update(100, 0, true, 0);
        assert(!results.didReset);
        assertLT(results.totalElapsedTime, 0);
    }

    // now clock it - should go to zero


    printf("finish this reset test\n");
   // assert(false);      // finish me
}

void testSeqClock()
{
    testOneShotInit();
    testOneShot2Ms();
    testClockInternal0();
    testClockExt1();
    testClockExtEdge();
    testClockInternalRunStop();
    testClockChangeWhileStopped();
    testSimpleReset();
    testSimpleResetIgnoreClock();
    testResetIgnoreClock();
    testRates();
    testNoNoteAfterReset();
}