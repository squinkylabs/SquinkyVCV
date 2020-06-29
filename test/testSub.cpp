
#include "tutil.h"
#include "asserts.h"

#include "Sub.h"
#include "TestComposite.h"


using Comp = Sub<TestComposite>;


extern bool _logvco;
/*
    auto ret = std::make_shared<T>();
    ret->init();
    auto icomp = ret->getDescription();
    for (int i = 0; i < icomp->getNumParams(); ++i) {
        auto param = icomp->getParam(i);
        ret->params[i].value = param.def;
    }
    */

static void testSub1()
{
    Comp sub;
    initComposite(sub);
    sub.step();
}

static void testSubLevel(bool sub, int vcoNumber, int side)
{
    assert(side >= 0 && side <= 1);
    assert(vcoNumber >= 0 && vcoNumber <= 1);       // only implemented these two values

    VoltageControlledOscillator<16, 16, rack::simd::float_4, rack::simd::int32_4> osc;
    osc.index = 0;
    int channels = (vcoNumber == 0) ? 1 : 2;
    const float deltaTime = 1.f / 44100.f;
    osc.setupSub(channels, rack::simd::float_4(2), rack::simd::int32_4(4), rack::simd::int32_4(4));
    std::function<float()> lambda = [&osc, deltaTime, sub, vcoNumber, side]() {
        osc.process(deltaTime, 0);
        return sub ? osc.sub(side)[vcoNumber] : osc.main()[vcoNumber];
    };


    auto stats = getSignalStats(10000, lambda);
    printf("test sub=%d, vco#=%d, side=%d\n", sub, vcoNumber, side);
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
    // this doesn't look right
    #if 0
    sub._get(0)._channels = 0;
    sub._get(0)._channels = 1;
    sub._get(0)._channels = 2;
    sub._get(0)._channels = 3;
    #else
    sub._get(0)._channels = 0;
    sub._get(1)._channels = 0;
    sub._get(2)._channels = 0;
    sub._get(3)._channels = 0;
    #endif
}


/**
 * conclusions:
 * 
 * when vec <> memory,  vec[0] is the lowest (first) memory address
 * float_4(a,b,c,d) will put a in vec[0];
 * float_vec1 > float_vec2 puts 1 is the sign bit of the elements, but does not set all the bits.
 * movemask(t,f,f,f) returns 1 (as one would expect)
 * float_4::mask doesn't work.
 * 
 */
#if 0
static void experiment()
{
    float test[] = {1,2,3,4,5,6,7,8}; 
    float_4 a = float_4::load(test);
    printf("test arrary from mem: %s\n", toStr(a).c_str());
    printf("a[0] = %f\n", a[0]);

    //	int halfMask = simd::movemask((0 < halfCrossing) & (halfCrossing <= 1.f));
    float_4 b(0);
    b[0] = 2;
    printf("test vec = %s\n", toStr(b).c_str());
    float_4 c = (b > 1);
     printf("test (b > 1) = %s\n", toStr(c).c_str());
    int32_4 d = c;
    printf("test (b > 1) hex = %s\n", toStrHex(d).c_str());
    printf("as mask: %s\n", toStrM(c).c_str());

    int mask = simd::movemask(c);
    printf("movemask %x\n", mask);

    float_4 e(1,2,3,4);
    printf("made from discrete ctor: %s\n", toStr(e).c_str());

    float_4 allone = float_4::mask();
    int32_4 allone2 = int32_4::mask();
    float_4 allone3 = allone2;
    printf("simd::mask = %s\n", toStrHex(allone ).c_str());
    printf("simd::mask2 = %s\n", toStr(allone2).c_str());
    printf("simd::mask3 = %s\n", toStr(allone3).c_str());

    mask = simd::movemask(allone2);
    printf("all mask bits int = %x\n", mask);
    mask = simd::movemask(allone3);
    printf("all mask bits float = %x\n", mask);
    exit(0);
}
#endif

static void testChannels()
{
    Comp sub;
    initComposite(sub);

    sub.inputs[Comp::VOCT_INPUT].channels = 0;
    resetChan(sub);
    sub.stepm();
    sub.stepn();
    assertEQ(sub._get(0)._channels, 2);
    assertEQ(sub._get(1)._channels, 0);
    assertEQ(sub._get(2)._channels, 0);
    assertEQ(sub._get(3)._channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 1;
    resetChan(sub);
    sub.stepm();
    sub.stepn();
    assertEQ(sub._get(0)._channels, 2);
    assertEQ(sub._get(1)._channels, 0);
    assertEQ(sub._get(2)._channels, 0);
    assertEQ(sub._get(3)._channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 2;
    resetChan(sub);
    sub.stepm();
    sub.stepn();
    assertEQ(sub._get(0)._channels, 4);
    assertEQ(sub._get(1)._channels, 0);
    assertEQ(sub._get(2)._channels, 0);
    assertEQ(sub._get(3)._channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 3;
    resetChan(sub);
    sub.stepm();
    sub.stepn();
    assertEQ(sub._get(0)._channels, 4);
    assertEQ(sub._get(1)._channels, 2);
    assertEQ(sub._get(2)._channels, 0);
    assertEQ(sub._get(3)._channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 4;
    resetChan(sub);
    sub.stepm();
    sub.stepn();
    assertEQ(sub._get(0)._channels, 4);
    assertEQ(sub._get(1)._channels, 4);
    assertEQ(sub._get(2)._channels, 0);
    assertEQ(sub._get(3)._channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 5;
    resetChan(sub);
    sub.stepm();
    sub.stepn();
    assertEQ(sub._get(0)._channels, 4);
    assertEQ(sub._get(1)._channels, 4);
    assertEQ(sub._get(2)._channels, 2);
    assertEQ(sub._get(3)._channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 6;
    resetChan(sub);
    sub.stepm();
    sub.stepn();
    assertEQ(sub._get(0)._channels, 4);
    assertEQ(sub._get(1)._channels, 4);
    assertEQ(sub._get(2)._channels, 4);
    assertEQ(sub._get(3)._channels, 0);

    sub.inputs[Comp::VOCT_INPUT].channels = 7;
    resetChan(sub);
    sub.stepm();
    sub.stepn();
    assertEQ(sub._get(0)._channels, 4);
    assertEQ(sub._get(1)._channels, 4);
    assertEQ(sub._get(2)._channels, 4);
    assertEQ(sub._get(3)._channels, 2);

    sub.inputs[Comp::VOCT_INPUT].channels = 8;
    resetChan(sub);
    sub.stepm();
    sub.stepn();
    assertEQ(sub._get(0)._channels, 4);
    assertEQ(sub._get(1)._channels, 4);
    assertEQ(sub._get(2)._channels, 4);
    assertEQ(sub._get(3)._channels, 4);
}

static void testSub2()
{
    printf("enter test sub 2\n"); fflush(stdout);
  //  Comp sub;
    std::vector<std::shared_ptr<Comp>> comps;
    for (int i=0; i<10; ++i) {
        std::shared_ptr<Comp> sub = std::make_shared<Comp>();
   
       // sub->onSampleRateChange();
        sub->init();
        initComposite(*sub);
        comps.push_back(sub);
    }
    for (size_t j = 0; j<comps.size(); ++j) {
        auto sub = comps[j];
        for (int i=0; i<1000000; ++i) {
            sub->step();
        }
    }
     printf("exit test sub 2\n"); fflush(stdout);
}

void testSub()
{
  //  experiment();
    testChannels();
    testSub1();
    testSub2();

    testSubLevel(false, 0, 0);
    testSubLevel(false, 1, 0);
    printf("can't test sub level yet - problems\n");
    printf("what about testing all waveforms\n");
   // testSubLevel(true, 0, 0);
  //  testSubLevel(true, 1, 0);

    testSubLevel(false, 0, 1);
    testSubLevel(false, 1, 1);
  //  testSubLevel(true, 0, 1);
  //  testSubLevel(true, 1, 1);
}