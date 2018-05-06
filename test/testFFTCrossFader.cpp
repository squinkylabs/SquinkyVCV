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
static void test3()
{
    Tester test(4, 10);
    for (int i = 0; i < 10; ++i) {
        test.messages[0]->dataBuffer->set(i, 9);    
        test.messages[1]->dataBuffer->set(i, 0);     
    }

    // put both in, to cross fade
    NoiseMessage* t = test.f.acceptData(test.messages[0].get());
    assertEQ(t, 0);
    t = test.f.acceptData(test.messages[1].get());
    assertEQ(t, 0);

    int emptyCount = 0;

    // play buffer once
    float expected[] = {9, 6, 3, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 10; ++i) {
        float x = 5;
        t = test.f.step(&x);
        if (t) {
            ++emptyCount;
        }
        printf("i=%d, x = %f\n", i, x);
        assertEQ(x, expected[i]);
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



void testFFTCrossFader()
{
    assertEQ(FFTDataReal::_count, 0);
    test0();
    test1();
    test2();
    test3();
    assertEQ(FFTDataReal::_count, 0);
}