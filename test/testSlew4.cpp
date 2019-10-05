#include "TestComposite.h"
#include "Slew4.h"
#include "TestComposite.h"

#include "asserts.h"

using Slew = Slew4<TestComposite>;

static void clearConnections(Slew& slew)
{
    for (int i = 0; i < 8; ++i) {
        slew.inputs[Slew::INPUT_TRIGGER0 + i].channels = 0;
        slew.inputs[Slew::INPUT_AUDIO1 + i].channels = 0;
        slew.outputs[Slew::OUTPUT0 + i].channels = 0;
        slew.outputs[Slew::OUTPUT_MIX0 + i].channels = 0;
    }
}


static void activateGateInputs(Slew& slew)
{
    for (int i = 0; i < 8; ++i) {
        slew.inputs[Slew::INPUT_TRIGGER0 + i].channels = 1;
    }
}


// one channel at a time, test that channel will pass gate,
// and the others are not affected.
// set for fast rise and fall
static void testTriggers(int outputNumber)
{
    Slew slew;
    slew.init();
    clearConnections(slew);
    activateGateInputs(slew);

    slew.params[Slew::PARAM_RISE].value = -5;
    slew.params[Slew::PARAM_FALL].value = -5;
    slew.inputs[Slew::INPUT_TRIGGER0 + outputNumber].value = 10;       // trigger channel under test
    slew.inputs[Slew::INPUT_TRIGGER0 + outputNumber].channels = 1;
    slew.step();

    for (int i = 0; i < 8; ++i) {
        auto value = slew.outputs[Slew::OUTPUT0 + i].value;
        if (outputNumber == i) {
             // 1.1 is pretty fast, just picked it by observing lag
            assertClose(value, 1.1, .1);
        } else {
            assertEQ(value, 0);
        }
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

    for (int i = 0; i < 8; ++i) {
        slew.outputs[Slew::OUTPUT_MIX0 + i].channels = (i == outputNumber) ? 1 : 0;       // patch the output under test
    }

    if (outputNumber == 1) {
        assert(!slew.outputs[Slew::OUTPUT_MIX0].isConnected());
    }

    // all the trigger inputs being driven at the same time
    for (int i = 0; i < 8; ++i) {
        slew.inputs[Slew::INPUT_TRIGGER0 + i].channels = 1;
        slew.inputs[Slew::INPUT_TRIGGER0 + i].value = 10;       // trigger all
    }

    slew.step();

    for (int i = 0; i < 8; ++i) {
        const float value = slew.outputs[Slew::OUTPUT_MIX0 + i].value;
        if (outputNumber == i) {
             // we expect the one patched output to have the sum of all above it.
            float expected = 1.19f * (i + 1);
            assertClose(value, expected, .1);
        } else {
            assertEQ(value, 0);
        }
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
    bool ret = slew.outputs[Slew::OUTPUT0 + gateOut].value > 1;         // min rise, fall, 1.19 (as above)
    return ret;
}

static void testGateInputs()
{
    // straight through
    for (int i = 0; i < 8; ++i) {
        Slew slew;
        init(slew);
        slew.inputs[Slew::INPUT_TRIGGER0 + i].channels = 1;
        assert(gateInputTriggersOutput(slew, i, i));
    }

    // un-patched, flows down via normals
    for (int i = 0; i < 8; ++i) {
        Slew slew;
        init(slew);

         // patch the first input only
        for (int j = 0; j < 8; ++j) {
            slew.inputs[Slew::INPUT_TRIGGER0 + j].channels = (j == 0) ? 1 : 0;
        }     
        assert(gateInputTriggersOutput(slew, 0, i));
    }

    {
        Slew slew;
        init(slew);
        slew.inputs[Slew::INPUT_TRIGGER0].channels = 1;
        slew.inputs[Slew::INPUT_TRIGGER1].channels = 1;
        assert(!gateInputTriggersOutput(slew, 0, 1));
    }

    {
        Slew slew;
        init(slew);
        slew.inputs[Slew::INPUT_TRIGGER0].channels = 1;
        slew.inputs[Slew::INPUT_TRIGGER1].channels = 1;
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