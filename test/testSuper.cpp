
#include "Super.h"
#include "TestComposite.h"
#include "asserts.h"

using Comp = Super<TestComposite>;

static void test0()
{
    Comp super;
    super.init();
    super.step();

}

static void testOutput(bool stereo, int mode, int channel = 0)
{
    assert(mode >= 0 && mode <= 2);
    Comp super;
    super.init();
    super.params[Comp::CLEAN_PARAM].value = float(mode);

   // assert(channel == 0);

    // hook up the outputs
    super.outputs[Comp::MAIN_OUTPUT_LEFT].channels = 1;
    super.outputs[Comp::MAIN_OUTPUT_RIGHT].channels = 1;

    super.inputs[Comp::CV_INPUT].channels = channel + 1;

    for (int i = 0; i < 50; ++i) {
        super.step();
    }
    super.step();
    assertEQ(super.outputs[Comp::MAIN_OUTPUT_LEFT].getChannels(), channel + 1);
    assertNE(super.outputs[Comp::MAIN_OUTPUT_LEFT].getVoltage(channel), 0);
    assertNE(super.outputs[Comp::MAIN_OUTPUT_RIGHT].getVoltage(channel), 0);
}

using CompPtr = std::shared_ptr<Comp>;

static CompPtr makeSaw()
{
    CompPtr ret = std::make_shared<Comp>();
    ret->init();
    ret->params[Comp::CLEAN_PARAM].value = float(0);

   // assert(channel == 0);

    // hook up the outputs
    ret->outputs[Comp::MAIN_OUTPUT_LEFT].channels = 1;
    ret->outputs[Comp::MAIN_OUTPUT_RIGHT].channels = 1;

    // hook up the mod inputs as mono
    ret->inputs[Comp::FM_INPUT].channels = 1;
    ret->inputs[Comp::TRIGGER_INPUT].channels = 1;
    ret->inputs[Comp::DETUNE_INPUT].channels = 1;
    ret->inputs[Comp::MIX_INPUT].channels = 1;
    ret->inputs[Comp::CV_INPUT].channels = 1;
    return ret;

}

static void stepn(CompPtr comp, int n)
{
    for (int i = 0; i < n; ++i) {
        comp->step();
    }
}

static void testRun(int numChannels)
{
   // printf("TestRun %d\n", numChannels);
    CompPtr comp = makeSaw();
    comp->inputs[Comp::CV_INPUT].channels = numChannels;
    stepn(comp, 16);
    for (int i = 0; i < 16; ++i) {
        SuperDsp& dsp = comp->_getDsp(i);

        const bool isMono = numChannels < 2;            // both 0 and one should run first chanel
        const bool shouldRun = isMono ? (i == 0) : (i < numChannels);

        const int expectedStep = shouldRun ? 16 : 0;
        const int expectedPh = shouldRun ? 4 : 0;
        assertEQ(dsp._stepCalls, expectedStep);
        assertEQ(dsp._updatePhaseIncCalls, expectedPh);
    }
}

static void testRun()
{
    testRun(1);
    testRun(2);
    testRun(0);
}

#if 0 // just for debugging
static void testFM()
{
    printf("\n---- TestFM\n");
    CompPtr comp = makeSaw();
     comp->inputs[Comp::CV_INPUT].channels = 2;
    stepn(comp, 16);
    printf("\n---- now turn up fm CV\n");
    comp->inputs[Comp::FM_INPUT].setVoltage(1);     // just set channel 0 FM
    comp->params[Comp::FM_PARAM].value = 1;         // and put the depth up
    SqInput& inp = comp->inputs[Comp::FM_INPUT];
    printf("fm input port test at %p\n", &inp);
    stepn(comp, 16);
    printf("test done\n");
    stepn(comp, 16);
}
#endif

void testSuper()
{
    test0();
    testOutput(false, 0);       // classic mono
    testOutput(false, 1);       // 4x mono
    testOutput(false, 2);       // 16x mono
    testOutput(true, 0);        // classic stereo
    testOutput(true, 1);        // 4x stereo
    testOutput(true, 2);        // 16x stereo
    testOutput(false, 0, 1);    // classic mono, channel2
    testOutput(false, 0, 3);    // classic mono, channel4
    testOutput(true, 2, 3);     // clean stereo, channel4

    testRun();
   // testFM();
}