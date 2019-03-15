
#include "SeqClock.h"
#include "asserts.h"


//  SeqClock::update(int samplesElapsed, float externalClock, float runStop, float reset)
// test internal clock
static void testClock0()
{
    const int sampleRateI = 44100;
    const float sampleRate = float(sampleRateI);
    const float sampleTime = 1.f / sampleRate;

    SeqClock ck;
    ck.setup(0, 120, sampleTime);       // internal clock

    // now clock by one second
    double elapsed = ck.update(sampleRateI, 0, 0, 0);

    // quarter note = half second at 120,
    // so one second = 2q
    assertEQ(elapsed, 2.0);

    elapsed = ck.update(sampleRateI, 0, 0, 0);
    assertEQ(elapsed, 4.0);

    ck.reset();
    ck.setup(0, 240, sampleTime);       // internal clock
    elapsed = ck.update(sampleRateI * 10, 0, 0, 0);
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
        double x = ck.update(55, 0, 0, 0);        // low clock
        assertEQ(x, 0);
    }

    // count home much metric time comes back
    double x = ck.update(55, 10, 0, 0);
    assertEQ(x, metricTimePerClock);

}

static void testClock1()
{
    testClockExt(5, 1.0);
    testClockExt(4, 1.0 / 2.0);
    testClockExt(3, 1.0 / 4.0);
    testClockExt(2, 1.0 / 8.0);
    testClockExt(1, 1.0 / 16.0);
}

void testSeqClock()
{
    testClock0();
    testClock1();
}