
#ifndef _MSC_VER 

#include "F2.h"
#include "F2_Poly.h"
#include "F4.h"

#include "TestComposite.h"

#include "tutil.h"
#include "asserts.h"



using Comp2_Scalar = F2<TestComposite>;
using Comp2_Poly = F2_Poly<TestComposite>;
using Comp4 = F4<TestComposite>;

template <class T>
static void testF2Fc(float fcParam, float cv, float expectedFcGain)
{
    auto setup = [fcParam, cv](T& comp) {
        comp.params[T::FC_PARAM].value = fcParam;
        comp.inputs[T::FC_INPUT].setVoltage(cv, 0);
    };

    auto validate = [expectedFcGain](T& comp) {
        assertClosePct(comp._params1()._fcGain(), expectedFcGain, 10);
    };
    testArbitrary<T>(setup, validate);
}


static void testF2Fc_Poly(float fcParam, float cv, float expectedFcGain)
{
    // printf("\n---- testF2Fc_Poly %f, %f, %f\n", fcParam, cv, expectedFcGain); fflush(stdout);
    auto setup = [fcParam, cv](Comp2_Poly& comp) {
        comp.params[Comp2_Poly::FC_PARAM].value = fcParam;
        comp.inputs[Comp2_Poly::FC_INPUT].setVoltage(cv, 0);
        comp.inputs[Comp2_Poly::AUDIO_INPUT].channels = 4;
    };

    auto validate = [expectedFcGain](Comp2_Poly& comp) {
       simd_assertClosePct(comp._params1()._fcGain(), float_4(expectedFcGain), 10);
    };
    testArbitrary<Comp2_Poly>(setup, validate);
}


static void testF2Q_Poly(float qParam, float qcv, float expectedQGain)
{
    auto setup = [qParam, qcv](Comp2_Poly& comp) {
        comp.params[Comp2_Poly::Q_PARAM].value = qParam;
        comp.inputs[Comp2_Poly::Q_INPUT].setVoltage(qcv, 0);
        comp.inputs[Comp2_Poly::AUDIO_INPUT].channels = 4;
    };

    auto validate = [expectedQGain](Comp2_Poly& comp) {
        simd_assertClosePct(comp._params1()._qGain(), float_4(expectedQGain), 10);
    };
    testArbitrary<Comp2_Poly>(setup, validate);
}

template <class T>
static void testF2Q(float qParam, float qcv, float expectedFcGain)
{
    auto setup = [qParam, qcv](T& comp) {
        comp.params[T::Q_PARAM].value = qParam;
        comp.inputs[T::Q_INPUT].setVoltage(qcv, 0);
    };

    auto validate = [expectedFcGain](T& comp) {
        assertClosePct(comp._params1()._qGain(), expectedFcGain, 10);
    };
    testArbitrary<T>(setup, validate);
}

static void testF2R(float rParam, float rcv, float fcParam, float expectedFcGain1, float expectedFcGain2)
{
    auto setup = [rParam, rcv, fcParam](Comp2_Scalar& comp) {
        comp.params[Comp2_Scalar::R_PARAM].value = rParam;
        comp.params[Comp2_Scalar::FC_PARAM].value = fcParam;
        comp.inputs[Comp2_Scalar::R_INPUT].setVoltage(rcv, 0);
        comp.inputs[Comp2_Scalar::FC_INPUT].setVoltage(rcv, 0);
        comp.inputs[Comp2_Scalar::AUDIO_INPUT].channels = 4;
    };

    auto validate = [expectedFcGain1,expectedFcGain2 ](Comp2_Scalar& comp) {
        assertClosePct(comp._params1()._fcGain(), expectedFcGain1, 10);
        assertClosePct(comp._params2()._fcGain(), expectedFcGain2, 10);
    };
    testArbitrary<Comp2_Scalar>(setup, validate);
}


// All of these "expected" values are just harvested known goods.
// but this will let us test we break anything when we make it poly/simd
static void testF2Fc()
{
    testF2Fc<Comp2_Scalar>(0, 0, .00058);
    testF2Fc<Comp2_Scalar>(0, -10, .00058);
    testF2Fc<Comp2_Scalar>(10, 0, .6);           // hmm... we are losing the top of the range - should scale it down below .5
    testF2Fc<Comp2_Scalar>(10, 10, .6);
    testF2Fc<Comp2_Scalar>(5, 0, .0186377);
}

static void testF2Fc_Poly()
{
    testF2Fc_Poly(0, 0, .00058);
    testF2Fc_Poly(0, -10, .00058);
    testF2Fc_Poly(10, 0, .6);           // hmm... we are losing the top of the range - should scale it down below .5
    testF2Fc_Poly(10, 10, .6);
}

static void testF2Q()
{
    testF2Q<Comp2_Scalar>(0, 0, 2);
    testF2Q<Comp2_Scalar>(0, -10, 2);
    testF2Q<Comp2_Scalar>(1, 0, .92);
    testF2Q<Comp2_Scalar>(10, 0, .0099);
    testF2Q<Comp2_Scalar>(10, 10, .0099);
}

static void testF2Q_Poly()
{
    testF2Q_Poly(0, 0, 2);
    testF2Q_Poly(0, -10, 2);
    testF2Q_Poly(1, 0, .92);
    testF2Q_Poly(10, 0, .0099);
    testF2Q_Poly(10, 10, .0099);
}

static void testF2R()
{
   //  void testF2R(float rParam, float rcv, float fcParam, float expectedFcGain1, float expectedFcGain2)
    testF2R(0, 0, 5, .0186377, .0186377);
    testF2R(5, 0, 5, .0058705, .0591709);
    testF2R(10, 0, 2.5, .00032, .0332);
}

static void testF4Fc()
{
    testF2Fc<Comp4>(0, 0, .00058);
}

static void testPolyChannelsF2()
{
    testPolyChannels<Comp2_Poly>(Comp2_Poly::AUDIO_INPUT, Comp2_Poly::AUDIO_OUTPUT, 16);
}

void testFilterComposites()
{
    testF2Fc();
    testF2Fc_Poly();
    
    testF2Q();
    testF2Q_Poly();

    testF2R();

    printf("please add back f4 compostite tests\n");
   // testF4Fc();
   //void testPolyChannels(int  inputPort, int outputPort, int numChannels)
    testPolyChannelsF2();
}

#endif