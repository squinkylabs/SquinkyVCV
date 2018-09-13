

#include "FractionalDelay.h"
#include "asserts.h"


static void test0()
{
    FractionalDelay f(100);
    f.setDelay(50);
    float x = f.run(1);
    assertEQ(x, 0);
}

static void test1()
{

    FractionalDelay f(100);
    f.setDelay(10);
    float x = 0;
    for (int i = 0; i < 20; ++i) {
        x = f.run(1);
    }
    assertEQ(x, 1);
}

void testDelay()
{
    test0();
}