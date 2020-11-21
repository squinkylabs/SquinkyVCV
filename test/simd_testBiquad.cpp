/**
 *
 */

#include "asserts.h"
#include <stdio.h>


#include "ButterworthFilterDesigner.h"
#include "BiquadFilter.h"
#include "BiquadFilter.h"
#include "BiquadParams.h"
#include "BiquadParams.h"
#include "BiquadState.h"
#include "IIRDecimator.h"

#include <simd/vector.hpp>
#include <simd/functions.hpp>

using float_4 = rack::simd::float_4;

template<typename T, int N>
static void testState_0()
{
    BiquadState<T, N> p;
    for (int i = 0; i < N; ++i) {
       // assert(p.z0(i) == 0);
        simd_assertEQ(p.z0(i), float_4::zero());
        simd_assertEQ(p.z1(i), float_4::zero());
    }
    p.z0(0) = 5;
    p.z1(0) = 6;

    simd_assertEQ(p.z0(0), float_4(5));
    simd_assertEQ(p.z1(0), float_4(6));


    if (N > 1) {
        p.z0(N - 1) = 55;
        p.z1(N - 1) = 66;
        simd_assertEQ(p.z0(N-1), float_4(55));
        simd_assertEQ(p.z1(N-1), float_4(66));
    }

    simd_assertEQ(p.z0(0), float_4(5));
    simd_assertEQ(p.z1(0), float_4(6));
}

template<typename T, int N>
static void testParam_0()
{
    BiquadParams<T, N> p;
    for (int i = 0; i < N; ++i) {
        simd_assertEQ(p.A1(0), float_4(0));
        simd_assertEQ(p.A2(0), float_4(0));
        simd_assertEQ(p.B0(0), float_4(0));
        simd_assertEQ(p.B1(0), float_4(0));
        simd_assertEQ(p.B2(0), float_4(0));
    }

    p.A1(0) = 1;
    p.A2(0) = 2;
    p.B0(0) = 10;
    p.B1(0) = 11;
    p.B2(0) = 12;

    simd_assertEQ(p.A1(0), float_4(1));
    simd_assertEQ(p.A2(0), float_4(2));
    simd_assertEQ(p.B0(0), float_4(10));
    simd_assertEQ(p.B1(0), float_4(11));
    simd_assertEQ(p.B2(0), float_4(12));

    if (N > 1) {
        p.A1(N - 1) = 111;
        p.A2(N - 1) = 112;
        p.B0(N - 1) = 1110;
        p.B1(N - 1) = 1111;
        p.B2(N - 1) = 1112;

        simd_assertEQ(p.A1(N-1), float_4(111));
        simd_assertEQ(p.A2(N-1), float_4(112));
        simd_assertEQ(p.B0(N-1), float_4(1110));
        simd_assertEQ(p.B1(N-1), float_4(1111));
        simd_assertEQ(p.B2(N-1), float_4(1112));
    }

    simd_assertEQ(p.A1(0), float_4(1));
    simd_assertEQ(p.A2(0), float_4(2));
    simd_assertEQ(p.B0(0), float_4(10));
    simd_assertEQ(p.B1(0), float_4(11));
    simd_assertEQ(p.B2(0), float_4(12));
}


template<typename T>
static void test2()
{
    BiquadParams<T, 3> p;
    ButterworthFilterDesigner<T>::designSixPoleLowpass(p, T(.1));
    BiquadState<T, 3> s;
    T d = BiquadFilter<T>::run(0, s, p);
    (void) d;
}



// test that filter designer does something (more than just generate zero
template<typename T>
static void testBasicDesigner2()
{
    BiquadParams<T, 3> p;
    ButterworthFilterDesigner<T>::designSixPoleLowpass(p, T(.1));
    simd_assertNE(p.A1(0), float_4::zero());
    simd_assertNE(p.A2(0), float_4::zero());
    simd_assertNE(p.B0(0), float_4::zero());
    simd_assertNE(p.B1(0), float_4::zero());
    simd_assertNE(p.B2(0), float_4::zero());
}

static void testCompare(int index) {
    BiquadParams<float_4, 3> vectorParams; 
    BiquadParams<float, 3> scalarParams;

    BiquadState<float_4, 3> vectorState; 
    BiquadState<float, 3> scalarState; 
   
    float scalarFreq = .1f;
    float_4 vectorFreq = float_4(.00001f);

    vectorFreq[index] = scalarFreq;

    ButterworthFilterDesigner<float>::designSixPoleLowpass(scalarParams, scalarFreq);
    ButterworthFilterDesigner<float_4>::designSixPoleLowpass(vectorParams, vectorFreq);

    for (int i = 0; i < 100; ++i) {
        float x = BiquadFilter<float>::run(1, scalarState, scalarParams);
        float_4 x_4 = BiquadFilter<float_4>::run(1, vectorState, vectorParams);

        for (int j=0; j<4; ++j) {
            if (j == index) {
                assertClose(x, x_4[j], .0001);
            } else {
                assertLT(x_4[j], x / 2);
            }
        }
    }
}

static void testCompare()
{
    for (int i=0; i<4; ++i) {
        testCompare(i);
    }
}

#if 0
// test that filter does something
template<typename T>
static void testBasicFilter2()
{
    BiquadParams<T, 3> params;
    BiquadState<T, 3> state;
    ButterworthFilterDesigner<T>::designSixPoleLowpass(params, T(.1));

    T lastValue = -1;

    // the first five values of the step increase
    for (int i = 0; i < 100; ++i) {
        T temp = BiquadFilter<T>::run(1, state, params);
        if (i < 5) {
            // the first 5 are strictly increasing and below 1
           // assert(temp < 1);
           // assert(temp > lastValue);
            simd_assertLT(temp, float_4(1));
            simd_assertGT(temp, lastValue);
        } else if (i < 10) {
            // the next are all overshoot
            //assert(temp > 1 && temp < 1.05);
            simd_assertGT(temp, float_4(1));
            simd_assertLT(temp, float_4(1.05));
        } else if (i > 50) {
            //settled
            //assert(temp > .999 && temp < 1.001);
            simd_assertGT(temp, float_4(.999));
            simd_assertLT(temp, float_4(1.001));
        }

        lastValue = temp;
    }
    const T val = BiquadFilter<T>::run(1, state, params);
    (void) val;

}


// test that filter does something
template<typename T>
static void testBasicFilter3()
{
    BiquadParams<T, 2> params;
    BiquadState<T, 2> state;
    ButterworthFilterDesigner<T>::designThreePoleLowpass(params, T(.1));

    T lastValue = 1;

    //the first five values of the step decrease (to -1)
    for (int i = 0; i < 100; ++i) {
        T temp = BiquadFilter<T>::run(1, state, params);

        if (i < 6) {
            // the first 5 are strictly increasing and below 1
            assert(temp > -1);
            assert(temp < lastValue);
        } else if (i < 10) {
            // the next are all overshoot
            assert(temp < -1);
            assert(temp > -1.1);
        } else if (i > 400) {
            //settled
            assert(temp < -.999 && temp > -1.001);
        }
        lastValue = temp;
    }
}
#endif


// test that the functions can be called
static void testBasicDecimator0()
{
    float_4 buffer[16];

    IIRDecimator<float_4> dec;
    dec.setup(16);
    dec.process(buffer);
}

// test 0 -> 0
static void testBasicDecimator1()
{
    float_4 buffer[16] = {0};

    IIRDecimator<float_4> dec;
    dec.setup(16);

    const float_4 x = dec.process(buffer);
    simd_assertEQ(x, float_4(0));
}



// test 10 -> 10
static void testBasicDecimator2()
{
    float_4 buffer[16] = {10 * 16};

    IIRDecimator<float_4> dec;
    dec.setup(16);

    float_4 x;
    for (int i = 0; i < 100; ++i) {
        x = dec.process(buffer);
    }
    simd_assertClose(x, float_4(10), .01);
    simd_assertSame(x);
}


void simd_testBiquad()
{
    testState_0<float_4, 8>();
    testParam_0<float_4, 4>();

    test2<float_4>();

    testBasicDesigner2<float_4>();
    testCompare();

    testBasicDecimator0();
    testBasicDecimator1();
    testBasicDecimator2();
#if 0
  testBasicFilter2<float_4>();
    testBasicDesigner2<float>();
    testBasicDesigner3<double>();
    testBasicDesigner3<float>();


    testBasicFilter2<float>();
    testBasicFilter3<double>();
    testBasicFilter3<float>();
    #endif

}