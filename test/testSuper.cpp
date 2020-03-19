
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

static void testOutput(bool stereo, int mode)
{
    assert(mode >= 0 && mode <= 2);
    Comp super;
    super.init();
    super.params[Comp::CLEAN_PARAM].value = float(mode);

    if (stereo) {
        super.outputs[Comp::MAIN_OUTPUT_LEFT].channels = 1;
        super.outputs[Comp::MAIN_OUTPUT_RIGHT].channels = 1;

        bool isStereo = super.outputs[Comp::MAIN_OUTPUT_RIGHT].isConnected() && super.outputs[Comp::MAIN_OUTPUT_LEFT].isConnected();
        assert(isStereo);
    }
    for (int i = 0; i < 50; ++i) {
        super.step();
    }
    assertNE(super.outputs[Comp::MAIN_OUTPUT_LEFT].getVoltage(0), 0);
    assertNE(super.outputs[Comp::MAIN_OUTPUT_RIGHT].getVoltage(0), 0);
}

void testSuper()
{
    test0();
    testOutput(false, 0);       // classic mono
    testOutput(false, 1);       // 4x mono
    testOutput(false, 2);       // 16x mono
    testOutput(true, 0);       // classic mono
    testOutput(true, 1);       // 4x mono
    testOutput(true, 2);       // 16x mono
}