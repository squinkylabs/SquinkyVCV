
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

static void testOutput()
{
    Comp super;
    super.init();
    for (int i = 0; i < 50; ++i) {
        super.step();
    }
    float x = super.outputs[Comp::MAIN_OUTPUT_LEFT].getVoltage(0);
    assertNE(x, 0);
}

void testSuper()
{
    test0();
    testOutput();
}