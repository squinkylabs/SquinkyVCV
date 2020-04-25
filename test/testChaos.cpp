
#include "ChaosKitty.h"
#include "TestComposite.h"
#include "asserts.h"

using Comp = ChaosKitty<TestComposite>;

void assertG(float g) {
    assert(g > 3.5);
    assert(g < 4);
}

static void test1()
{
    Comp kitty;
    const float g = kitty._getG();

    assertG(g);
}

void testChaos()
{
    test1();
}