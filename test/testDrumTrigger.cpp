
#include "DrumTrigger.h"
#include "TestComposite.h"

#include "asserts.h"

using DT = DrumTrigger<TestComposite>;

static void testInitialState()
{
    DT dt;
    dt.inputs[DT::GATE_INPUT].channels = 8;
    dt.step();
    for (int i = 0; i < numTriggerChannels; ++i) {
        assertLT(dt.outputs[DT::GATE0_OUTPUT + i].value, 1);
    }
}


static void test1Sub(float initCV)
{
    DT dt;
    dt.inputs[DT::GATE_INPUT].channels = 8;
    dt.inputs[DT::CV_INPUT].channels = 8;
   
    for (int i = 0; i < numTriggerChannels; ++i) {
        dt.inputs[DT::CV_INPUT].voltages[i] = initCV;
    }

    for (int i = 0; i < numTriggerChannels; ++i) {
        const float pitch = DT::base() + PitchUtils::semitone * i;
        dt.inputs[DT::CV_INPUT].voltages[i] = pitch;
        dt.inputs[DT::GATE_INPUT].voltages[i] = 10;
        dt.step();
        assertGT(dt.outputs[DT::GATE0_OUTPUT + i].value, 5);
        assertGT(dt.lights[DT::LIGHT0 + i].value, 5);

        // turn it off
        dt.inputs[DT::GATE_INPUT].voltages[i] = 0;
        dt.step();
        assertLT(dt.outputs[DT::GATE0_OUTPUT + i].value, 1);
        assertLT(dt.lights[DT::LIGHT0 + i].value, 1);
    }
}

static void test1()
{
    test1Sub(-5);
}

static void test2()
{
    test1Sub(0);
}

// feed an input that is out of current range.
static void testLess()
{
    DT dt;
    dt.inputs[DT::GATE_INPUT].channels = 1;

    for (int i = 0; i < numTriggerChannels; ++i) {
        dt.inputs[DT::CV_INPUT].voltages[i] = 0;
        dt.inputs[DT::GATE_INPUT].voltages[i] = 0;
    }

    const int i = 2;
    const float pitch = DT::base() + PitchUtils::semitone * i;
    dt.inputs[DT::CV_INPUT].voltages[i] = pitch;
    dt.inputs[DT::GATE_INPUT].voltages[i] = 10;
    dt.step();

    // should not respond to out of range port number
    assertLT(dt.outputs[DT::GATE0_OUTPUT + i].value, 1);
}

void testDrumTrigger()
{
    testInitialState();
    test1();
    test2();
    testLess();
}