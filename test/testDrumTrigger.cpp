
#include "DrumTrigger.h"
#include "TestComposite.h"

#include "asserts.h"

using DT = DrumTrigger<TestComposite>;

static void testInitialState()
{
    DT dt;
    dt.inputs[DT::GATE_INPUT].channels = 8;
    dt.step();
    for (int i = 0; i < DT::numChannels; ++i) {
        assertLT(dt.outputs[DT::GATE0_OUTPUT + i].value, 1);
    }
}

static void test1()
{
    DT dt;
    dt.inputs[DT::GATE_INPUT].channels = 8;
   
    for (int i = 0; i < DT::numChannels; ++i) {
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

void testDrumTrigger()
{
    testInitialState();
    test1();
}