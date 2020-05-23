
#include "tutil.h"
#include "asserts.h"

#include "Sub.h"
#include "TestComposite.h"


using Comp = Sub<TestComposite>;


extern bool _logvco;
#if 0
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

#endif
static void testSub1()
{
    Comp sub;
    initComposite(sub);
    sub.step();
}

static void testSubLevel(bool sub)
{
    VoltageControlledOscillator<16, 16, float_4, int32_4> osc;
    osc.channels = 1;
    const float deltaTime = 1.f / 44100.f;
   //float_4 deltaTime( 1.f / 44100.f);
    osc.setPitch(2);
    osc.setSubDivisor(4);

    std::function<float()> lambda = [&osc, deltaTime, sub]() {
        osc.process(deltaTime, 0);
        return sub ? osc.sub()[0] : osc.saw()[0];
    };


    auto stats = getSignalStats(1000, lambda);
    printf("stats ret %f, %f, %f\n", std::get<0>(stats), std::get<1>(stats), std::get<2>(stats));

    if (sub) {
        assertClose(std::get<1>(stats), 1, .2);       // max
        assertClose(std::get<0>(stats), -1, .2);      // min
        assertClose(std::get<2>(stats), .1, .1);       // average
    } else {
        assertClose(std::get<1>(stats), 1, .1);       // max
        assertClose(std::get<0>(stats), -1.2, .1);      // min
        assertClose(std::get<2>(stats), .1, .1);       // average
    }
}

static void resetChan(Comp& sub) {
    sub._get(0).channels = 0;
    sub._get(0).channels = 1;
    sub._get(0).channels = 2;
    sub._get(0).channels = 3;
}

static void testChannels()
{
    Comp sub;
    initComposite(sub);

    sub.inputs[Comp::VOCT_INPUT].channels = 0;
    resetChan(sub);
    sub.stepn();
    assertEQ(sub._get(0).channels, 2);
    assertEQ(sub._get(1).channels, 0);
    assertEQ(sub._get(2).channels, 0);
    assertEQ(sub._get(3).channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 1;
    resetChan(sub);
    sub.stepn();
    assertEQ(sub._get(0).channels, 2);
    assertEQ(sub._get(1).channels, 0);
    assertEQ(sub._get(2).channels, 0);
    assertEQ(sub._get(3).channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 2;
    resetChan(sub);
    sub.stepn();
    assertEQ(sub._get(0).channels, 4);
    assertEQ(sub._get(1).channels, 0);
    assertEQ(sub._get(2).channels, 0);
    assertEQ(sub._get(3).channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 3;
    resetChan(sub);
    sub.stepn();
    assertEQ(sub._get(0).channels, 4);
    assertEQ(sub._get(1).channels, 2);
    assertEQ(sub._get(2).channels, 0);
    assertEQ(sub._get(3).channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 4;
    resetChan(sub);
    sub.stepn();
    assertEQ(sub._get(0).channels, 4);
    assertEQ(sub._get(1).channels, 4);
    assertEQ(sub._get(2).channels, 0);
    assertEQ(sub._get(3).channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 5;
    resetChan(sub);
    sub.stepn();
    assertEQ(sub._get(0).channels, 4);
    assertEQ(sub._get(1).channels, 4);
    assertEQ(sub._get(2).channels, 2);
    assertEQ(sub._get(3).channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 6;
    resetChan(sub);
    sub.stepn();
    assertEQ(sub._get(0).channels, 4);
    assertEQ(sub._get(1).channels, 4);
    assertEQ(sub._get(2).channels, 4);
    assertEQ(sub._get(3).channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 7;
    resetChan(sub);
    sub.stepn();
    assertEQ(sub._get(0).channels, 4);
    assertEQ(sub._get(1).channels, 4);
    assertEQ(sub._get(2).channels, 4);
    assertEQ(sub._get(3).channels, 2);

     sub.inputs[Comp::VOCT_INPUT].channels = 8;
    resetChan(sub);
    sub.stepn();
    assertEQ(sub._get(0).channels, 4);
    assertEQ(sub._get(1).channels, 4);
    assertEQ(sub._get(2).channels, 4);
    assertEQ(sub._get(3).channels, 4);
}

void testSub()
{
   // testSub0();
    testChannels();
    testSub1();
    testSubLevel(false);
    testSubLevel(true);
    
}