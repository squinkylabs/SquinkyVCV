
#include "Slew4.h"
#include "TestComposite.h"

#include "asserts.h"

using Slew = Slew4<TestComposite>;

static void testTriggers(int outputNumber)
{
    Slew slew;
    slew.init();

    slew.params[Slew::PARAM_RISE].value = -5;
    slew.params[Slew::PARAM_FALL].value = -5;
    slew.inputs[Slew::INPUT_TRIGGER0 + outputNumber].value = 10;       // trigger channel under test
    slew.inputs[Slew::INPUT_TRIGGER0 + outputNumber].active = true;
    slew.step();

    for (int i = 0; i < 8; ++i) {
        const float expected = (outputNumber == i) ? 10.f : 0.f;
        assertEQ(slew.outputs[Slew::OUTPUT0 + outputNumber].value, 10);
    }

}

static void testTriggers()
{
    for (int i = 0; i < 8; ++i) {
        testTriggers(i);
    }
}

static void init(Slew& slew)
{
    slew.init();

    slew.params[Slew::PARAM_RISE].value = -5;
    slew.params[Slew::PARAM_FALL].value = -5;
    slew.params[Slew::PARAM_LEVEL].value = 1;
}

static void testMixedOutNormals(int outputNumber)
{
    Slew slew;
    init(slew);


    slew.outputs[Slew::OUTPUT_MIX0 + outputNumber].active = true;       // patch the output under test

    for (int i = 0; i < 8; ++i) {
        slew.inputs[Slew::INPUT_TRIGGER0 + outputNumber].active = true;
        slew.inputs[Slew::INPUT_TRIGGER0 + outputNumber].value = 10;       // trigger all
    }
    slew.step();

    for (int i = 0; i < 8; ++i) {
        // we expect the one patched output to have the sum of all above it.
        float expected = (outputNumber == i) ? 10.f : 0.f;
        expected *= (i + 1);
        assertEQ(slew.outputs[Slew::OUTPUT_MIX0 + outputNumber].value, 10);
    }
}

static void testMixedOutNormals()
{
    for (int i = 0; i < 8; ++i) {
        testMixedOutNormals(i);
    }
}

// gate input n triggers output m
bool gateInputTriggersOutput(Slew& slew, int gateIn, int gateOut)
{

    slew.inputs[Slew::INPUT_TRIGGER0 + gateIn].value = 10;
    slew.step();
    bool ret = slew.outputs[Slew::OUTPUT0 + gateOut].value > 5;
    return ret;
}

static void testGateInputs()
{
    // straight through
    for (int i = 0; i < 8; ++i) {
        Slew slew;
        init(slew);
        slew.inputs[Slew::INPUT_TRIGGER0 + i].active = true;
        assert(gateInputTriggersOutput(slew, i, i));
    }

    // un-patched, flows down via normals

    for (int i = 0; i < 8; ++i) {
        Slew slew;
        init(slew);
        slew.inputs[Slew::INPUT_TRIGGER0].active = true;       // patch the first input
        assert(gateInputTriggersOutput(slew, 0, i));
    }

    {
        Slew slew;
        init(slew);
        slew.inputs[Slew::INPUT_TRIGGER0].active = true;
        slew.inputs[Slew::INPUT_TRIGGER1].active = true;
        assert(!gateInputTriggersOutput(slew, 0, 1));
    }

    {
        Slew slew;
        init(slew);
        slew.inputs[Slew::INPUT_TRIGGER0].active = true;
        slew.inputs[Slew::INPUT_TRIGGER1].active = true;
        assert(!gateInputTriggersOutput(slew, 1, 0));
    }


}


#include "LFNB.h"
static void testLFNB()
{
    LFNB<TestComposite> b;
    b.init();
    for (int i=0; i<100; ++i)
        b.step();
}

void testSlew4()
{
    testTriggers();
    testMixedOutNormals();
    testGateInputs();
    testLFNB();
}