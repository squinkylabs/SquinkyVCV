
#include <assert.h>
#include "asserts.h"

#include "ClockMult.h"
#include "TestComposite.h"
#include "Tremolo.h"


/**
 * During training, we should get no output. 
 * On first ref-clock after training, should go high
 */
static void test0()
{
    ClockMult cm;
    cm.setDivisor(1);
    const int period = 4;

    printf("test0, 0\n");
    cm.refClock();  // give it an external clock

    printf("test0, 1\n");
    // train with 4 ref clocks()
    for (int i = 0; i < period; ++i) {
        cm.sampleClock();
        assertEQ(cm.getSaw(), 0);
        assertEQ(cm.getMultipliedClock(), false);
        printf("test0, 2\n");

    }

    // send ref-clock now to set period at 4 and start clocking
    cm.refClock();     
    assertEQ(cm.getSaw(), 0);
    assertEQ(cm.getMultipliedClock(), true);
    assertClose(cm._getFreq(), .25f, .000001);
}


/**
 * Test synched saw output
 */
static void test1()
{
    ClockMult cm;
    cm.setDivisor(1);
    const int period = 4;

    printf("test0, 0\n");
    cm.refClock();  // give it an external clock

    printf("test0, 1\n");
    // train with 4 ref clocks()
    for (int i = 0; i < period; ++i) {
        cm.sampleClock();
        assertEQ(cm.getSaw(), 0);
        assertEQ(cm.getMultipliedClock(), false);
        printf("test0, 2\n");

    }

    // send ref-clock now to set period at 4 and start clocking
    cm.refClock();
    assertEQ(cm.getSaw(), 0);
    assertEQ(cm.getMultipliedClock(), true);
    assertClose(cm._getFreq(), .25f, .000001);

    for (int i = 0; i < period-1; ++i) {
        cm.sampleClock();
        printf("in loop, i=%d, saw=%f\n", i, cm.getSaw());
        assertEQ(cm.getSaw(), .25 * (i + 1));
        assertEQ(cm.getMultipliedClock(), true);
    }
}

/**
 * Test free running
 */
static void test2()
{
    ClockMult cm;
    cm.setDivisor(0);
    cm.setFreeRunFreq(.1f);

    assertEQ(cm.getSaw(), 0);
    for (int i = 0; i < 9; ++i) {
        cm.sampleClock();
        assertClose(cm.getSaw(), (i+1) * .1f, .0001);
    }
    cm.sampleClock();
    assertClose(cm.getSaw(), 0, .0001);
    for (int i = 0; i < 9; ++i) {
        cm.sampleClock();
        assertClose(cm.getSaw(), (i + 1) * .1f, .0001);
    }
}
void testClockMult()
{
    test0();
    test1();
    test2();
}