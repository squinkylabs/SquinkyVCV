#include "FFTCrossFader.h"
#include "ColoredNoise.h"
#include "asserts.h"

#include <vector>


class Tester
{
public:
    Tester(int crossFadeSize, int frameSize) :
        f(crossFadeSize)
    {
        for (int i = 0; i < 3; ++i) {
            std::shared_ptr<NoiseMessage> p = std::make_shared<NoiseMessage>(frameSize);
            messages.push_back(p);
        }
    }

    FFTCrossFader f;
    std::vector< std::shared_ptr<NoiseMessage> > messages;
};

// accepting data on empty should not return on
static void test0()
{
    Tester test(4, 10);
    assertEQ(test.messages[0]->dataBuffer->get(0), 0);
    assertEQ(test.messages[0]->dataBuffer->get(9), 0);

    NoiseMessage* t = test.f.acceptData(test.messages[0].get());
    assertEQ(t, 0);
}

//empty should return 0
static void test1()
{
    Tester test(4, 10);
    for (int i = 0; i < 20; ++i) {
        float x = 5;
        test.f.step(&x);
        assertEQ(x, 0);
    }
}

// one buff, should play it
static void test2()
{
    Tester test(4, 10);
    for (int i = 0; i < 10; ++i) {
        test.messages[0]->dataBuffer->set(i, float(i));
    }
    NoiseMessage* t = test.f.acceptData(test.messages[0].get());
    assertEQ(t, 0);

    // pluy buffer once
    for (int i = 0; i < 10; ++i) {
        float x = 5;
        test.f.step(&x);
        assertEQ(x, i);
    }

    //play it again.
    for (int i = 0; i < 10; ++i) {
        float x = 5;
        test.f.step(&x);
        assertEQ(x, i);
    }
}


// two buff, should crossfade
static void test3(bool testBuff0)
{
    Tester test(4, 10);

    // fill the buff to test with data, other one with zeros
    for (int i = 0; i < 10; ++i) {
        test.messages[0]->dataBuffer->set(i, testBuff0 ? 9.f : 0.f);
        test.messages[1]->dataBuffer->set(i, testBuff0 ? 0.f : 18.f);     
    }

    // put both in, to cross fade
    NoiseMessage* t = test.f.acceptData(test.messages[0].get());
    assertEQ(t, 0);
    t = test.f.acceptData(test.messages[1].get());
    assertEQ(t, 0);

    int emptyCount = 0;

    // play buffer once

    // buffer 0 full of 9, so should see fade 9..0
    float expected0[] = {9, 6, 3, 0, 0, 0, 0, 0, 0, 0};

    // buffer 0 all zero, 1 all 18, so should see 0..18
    float expected1[] = {0, 6, 12, 18, 18, 18, 18, 18, 18, 18};
    for (int i = 0; i < 10; ++i) {
        float x = 5;
        t = test.f.step(&x);
        if (t) {
            ++emptyCount;
        }
        const float expected = testBuff0 ? expected0[i] : expected1[i];
        assertEQ(x, expected);
    }

    //play it again.
    for (int i = 0; i < 10; ++i) {
        float x = 5;
       // test.f.step(&x);
        t = test.f.step(&x);
        if (t) {
            ++emptyCount;
        }
        const float expectedTail = testBuff0 ? 0.f : 18.f;
        assertEQ(x, expectedTail);
    }
    
    assertEQ(emptyCount, 1);
}

// extra buffer rejected
static void test4()
{
    Tester test(4, 10);
    NoiseMessage* t = test.f.acceptData(test.messages[0].get());
    assertEQ(t, 0);
    t = test.f.acceptData(test.messages[1].get());
    assertEQ(t, 0);
    t = test.f.acceptData(test.messages[2].get());
    assertNE(t, 0);
}


// test wrap-around case
static void test5()
{
    // fade of 4, buffer of 8
    Tester test(4, 8);

    // fill the buff to test with data, other one with zeros
    for (int i = 0; i < 8; ++i) {
        test.messages[0]->dataBuffer->set(i, 0.f);
        test.messages[1]->dataBuffer->set(i, float(i));
    }

    // put zero 0
    NoiseMessage* t = test.f.acceptData(test.messages[0].get());
    
    float x;
    // clock 6
    for (int i = 0; i < 6; ++i) {
        x = 5;
        t = test.f.step(&x);
        assertEQ(x, 0);         // 0
        assertEQ(t, 0);
    }

    // now start crossfade

    t = test.f.acceptData(test.messages[1].get());
    assertEQ(t, 0);
    
    // sample #6. start fade
    t = test.f.step(&x);
    assertEQ(x, 0);         // 0
    assertEQ(t, 0);

    // sample #7.  fade #2
    t = test.f.step(&x);
    assertClose(x, .3333333f, .0001);         // 0
    assertEQ(t, 0);

    // sample#8, fade #3
    t = test.f.step(&x);
    assertClose(x, 1.3333333f, .0001);         // 0
    assertEQ(t, 0);

    // sample#8, fade #4 (last), buff 0 (?) gets returned
    t = test.f.step(&x);
    assertClose(x, 3.f, .0001);         // 0
    assertNE(t, 0);

    // done fading
    t = test.f.step(&x);
    assertClose(x, 4.f, .0001);         // 0
    assertEQ(t, 0);

    t = test.f.step(&x);
    assertClose(x, 5.f, .0001);         // 0
    assertEQ(t, 0);

    t = test.f.step(&x);
    assertClose(x, 6.f, .0001);         // 0
    assertEQ(t, 0);

    t = test.f.step(&x);
    assertClose(x, 7.f, .0001);         // 0
    assertEQ(t, 0);

    t = test.f.step(&x);
    assertClose(x, 0.f, .0001);         // 0
    assertEQ(t, 0);
}

void testFFTCrossFader()
{
    assertEQ(FFTDataReal::_count, 0);
    test0();
    test1();
    test2();
    test3(true);
    test3(false);
    test4();
    test5();
    assertEQ(FFTDataReal::_count, 0);
}