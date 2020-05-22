
#include "asserts.h"

#include "Sub.h"
#include "TestComposite.h"
#include "tutil.h"

using Comp = Sub<TestComposite>;


extern bool _logvco;
static void testSub0()
{
    printf("test sub!!!\n");  
    VoltageControlledOscillator<16, 16, float_4> osc;

    const float deltaTime = 1.f / 44100.f;
    osc.setPitch(1);
    osc.channels = 1;
    for (int i=0; i<100; ++i) {
        osc.process(deltaTime, 0);
        auto x = osc.saw();
        printf("%f: from saw\n\n", x[0]);
    }
    fflush(stdout);
}

static void testSub1()
{
    Comp sub;
    initComposite(sub);
    sub.step();
}

void testSub()
{
   // testSub0();
    testSub1();
}