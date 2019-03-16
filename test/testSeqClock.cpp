
#include "SeqClock.h"
#include "asserts.h"


//  SeqClock::update(int samplesElapsed, float externalClock, float runStop, float reset)
// test internal clock
static void testClockInternal0()
{
    const int sampleRateI = 44100;
    const float sampleRate = float(sampleRateI);
    const float sampleTime = 1.f / sampleRate;

    SeqClock ck;
    ck.setup(0, 120, sampleTime);       // internal clock

    // now clock by one second
    double elapsed = ck.update(sampleRateI, 0, true, 0);

    // quarter note = half second at 120,
    // so one second = 2q
    assertEQ(elapsed, 2.0);

    elapsed = ck.update(sampleRateI, 0, true, 0);
    assertEQ(elapsed, 4.0);

    ck.reset();
    ck.setup(0, 240, sampleTime);       // internal clock
    elapsed = ck.update(sampleRateI * 10, 0, true, 0);
    assertEQ(elapsed, 40);
}

static void testClockExt(int rate, double metricTimePerClock)
{
    assertGT(rate, 0);
    assertLE(rate, 5);

    SeqClock ck;
    ck.setup(rate, 120, 100);       // internal clock

    // send one clock
    for (int i = 0; i < 10; ++i) {
        double x = ck.update(55, 0, true, 0);        // low clock
        assertEQ(x, 0);
    }

    // count home much metric time comes back
    double x = ck.update(55, 10, true, 0);
    assertEQ(x, metricTimePerClock);

}

static void testClockExt1()
{
    testClockExt(5, 1.0);
    testClockExt(4, 1.0 / 2.0);
    testClockExt(3, 1.0 / 4.0);
    testClockExt(2, 1.0 / 8.0);
    testClockExt(1, 1.0 / 16.0);
}

static void testClockExtEdge()
{
    const int rate = 5;
    const double metricTimePerClock = 1;
    SeqClock ck;
    ck.setup(rate, 120, 100);       // internal clock

    // send one clock (first low)
    for (int i = 0; i < 10; ++i) {
        double x = ck.update(55, 0, true, 0);        // low clock
        assertEQ(x, 0);
    }

    // then high once
    double x = ck.update(55, 10, true, 0);
    assertEQ(x, metricTimePerClock);
    
    // then high some more
    for (int i = 0; i < 10; ++i) {
        double x = ck.update(55, 10, true, 0);        // low clock
        assertEQ(x, metricTimePerClock);
    }

     // low more
    for (int i = 0; i < 10; ++i) {
        double x = ck.update(55, 0, true, 0);        // low clock
        assertEQ(x, metricTimePerClock);
    }

     // then high some more
    for (int i = 0; i < 10; ++i) {
        double x = ck.update(55, 10, true, 0);        // low clock
        assertEQ(x, 2 * metricTimePerClock);
    }
}


static void testClockInternalRunStop()
{
    const int sampleRateI = 44100;
    const float sampleRate = float(sampleRateI);
    const float sampleTime = 1.f / sampleRate;

    SeqClock ck;
    ck.setup(0, 120, sampleTime);       // internal clock

    // now clock by one second
    double elapsed = ck.update(sampleRateI, 0, true, 0);

    // quarter note = half second at 120,
    // so one second = 2q
    assertEQ(elapsed, 2.0);

    // now clock stopped, should not run
    elapsed = ck.update(sampleRateI, 0, 0, 0);
    assertEQ(elapsed, 2.0);

     // now on again, should run
    elapsed = ck.update(sampleRateI, 0, true, 0);
    assertEQ(elapsed, 4.0);
}

void testSeqClock()
{
    testClockInternal0();
    testClockExt1();
    testClockExtEdge();
    testClockInternalRunStop();
}