
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

void testSuper()
{
    test0();
}