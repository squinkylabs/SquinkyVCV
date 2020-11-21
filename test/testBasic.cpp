
#include "asserts.h"
#include "BasicVCO.h"


using MinBlep = rack::dsp::MinBlepGenerator<16, 16, float_4>; 
using MinBlepScalar = rack::dsp::MinBlepGenerator<16, 16, float>; 

static void testBasic0()
{
    MinBlep m;
    MinBlepScalar s;
}

static void testBasic1()
{
    MinBlepScalar s;

    float phase = 0;
    s.insertDiscontinuity(phase, 1);
    float x = s.process();
    printf("proc = %f\n", x);
}

static void testSub(MinBlepScalar& scaler, float phase) 
{
    scaler.insertDiscontinuity(phase, 1);
   // printf("\n--------- phase %f \n", phase);
    for (int i=0; i< 32; ++i) {
        const float x = scaler.process();
       // printf("blep[%d] = %f\n", i, x);
        assert(x > -1);
        assert(x < .5);
    }
    for (int i=0; i< 32; ++i) {
        const float x = scaler.process();
        assertEQ(x, 0);
    }
}

static void testBasic2()
{
    MinBlepScalar s;
    double delta = .0000001;

    double phase = -2.1;
    for (int i=0; phase <= 1.2; ++ i) {
        phase = -1 + i * delta;
        testSub(s, .1f);
    }
}

static void testBasic3()
{
    MinBlepScalar s;

    float phase = 0;
    s.insertDiscontinuity(phase, 1);
    float x = s.process();
    printf("proc3 = %f\n", x);
    s.insertDiscontinuity(phase, 1);
    x = s.process();
    printf("proc3b = %f\n", x);
}

// this test might be superfluous...
void testBasic()
{
#ifndef _MSC_VER 
    testBasic0();
    testBasic1();
    testBasic2();
    testBasic3();
#else
    printf("testBasic skipped for MS compiler: no minBlep\n");
#endif
}