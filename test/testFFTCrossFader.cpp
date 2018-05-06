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
    float expected0[] = {9, 6, 3, 0, 0, 0, 0, 0, 0, 0};
    float expected1[] = {18, 12, 6, 0, 0, 0, 0, 0, 0, 0};
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
        test.f.step(&x);
        t = test.f.step(&x);
        if (t) {
            ++emptyCount;
        }
        assertEQ(x, 0);
    }
    assertEQ(emptyCount, 1);
}

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



void testFFTCrossFader()
{
    assertEQ(FFTDataReal::_count, 0);
    test0();
    test1();
    test2();
    test3(true);
    test4();
    assertEQ(FFTDataReal::_count, 0);
}