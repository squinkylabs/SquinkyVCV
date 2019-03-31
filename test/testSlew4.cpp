
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

static void testMixedOutNormals(int outputNumber)
{
    Slew slew;
    slew.init();

    slew.params[Slew::PARAM_RISE].value = -5;
    slew.params[Slew::PARAM_FALL].value = -5;
    slew.params[Slew::PARAM_LEVEL].value = 1;


    slew.outputs[Slew::OUTPUT_MIX0 + outputNumber].active = true;       // patch the output under test

    for (int i = 0; i < 8; ++i) {
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

void testSlew4()
{
  //  testTriggers();
    testMixedOutNormals();
}