
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

#if 0 // I think this idea was dumb?
    if (stereo) {
        // hook up the outputs for sterwo
        super.outputs[Comp::MAIN_OUTPUT_LEFT].channels = channel + 1;
        super.outputs[Comp::MAIN_OUTPUT_RIGHT].channels = channel + 1;

        bool isStereo = super.outputs[Comp::MAIN_OUTPUT_RIGHT].isConnected() && super.outputs[Comp::MAIN_OUTPUT_LEFT].isConnected();
        assert(isStereo);
    }
#endif

    super.inputs[Comp::CV_INPUT].channels = channel + 1;

    for (int i = 0; i < 50; ++i) {
        super.step();
    }
    super.step();
    assertEQ(super.outputs[Comp::MAIN_OUTPUT_LEFT].getChannels(), channel + 1);
    assertNE(super.outputs[Comp::MAIN_OUTPUT_LEFT].getVoltage(channel), 0);
    assertNE(super.outputs[Comp::MAIN_OUTPUT_RIGHT].getVoltage(channel), 0);
}

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
}