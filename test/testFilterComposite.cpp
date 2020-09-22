
#ifndef _MSC_VER 

#include "F2.h"
#include "F4.h"

#include "TestComposite.h"

#include "tutil.h"
#include "asserts.h"



using Comp2 = F2<TestComposite>;
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


// All of these "expected" values are just harvested known goods.
// but this will let us test we break anything when we make it poly/simd
static void testF2Fc()
{
    testF2Fc<Comp2>(0, 0, .00058);
    testF2Fc<Comp2>(0, -10, .00058);
    testF2Fc<Comp2>(10, 0, .6);           // hmm... we are losing the top of the range - should scale it down below .5
    testF2Fc<Comp2>(10, 10, .6);
}

static void testF2Q()
{
    testF2Q<Comp2>(0, 0, 2);
    testF2Q<Comp2>(0, -10, 2);
    testF2Q<Comp2>(1, 0, .92);
    testF2Q<Comp2>(10, 0, .0099);
    testF2Q<Comp2>(10, 10, .0099);
}

static void testF4Fc()
{
    testF2Fc<Comp4>(0, 0, .00058);
}

void testFilterComposites()
{

    testF2Fc();
    
    testF2Q();
     printf("please add back f4 compostite tests\n");
   // testF4Fc();
}

#endif