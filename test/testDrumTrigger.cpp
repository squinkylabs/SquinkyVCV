
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
    assertLT(dt.outputs[DT::GATE0_OUTPUT].value, 1);

    // trigger a note on  output 0
    dt.inputs[DT::CV_INPUT].voltages[0] = DT::base();
    dt.inputs[DT::GATE_INPUT].voltages[0] = 10;

    dt.step();
    assertGT(dt.outputs[DT::GATE0_OUTPUT].value, 5);
    
   
}

void testDrumTrigger()
{
    testInitialState();
    test1();
}