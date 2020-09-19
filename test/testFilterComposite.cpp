
#ifndef _MSC_VER 

#include "F2.h"
#include "TestComposite.h"

#include "tutil.h"
#include "asserts.h"



using Comp2 = F2<TestComposite>;

static void testF2Fc(float fcParam, float cv, float expectedFcGain)
{
    auto setup = [fcParam, cv](Comp2& comp) {
        comp.params[Comp2::FC_PARAM].value = fcParam;
        comp.inputs[Comp2::FC_INPUT].setVoltage(cv, 0);
    };

    auto validate = [expectedFcGain](Comp2& comp) {
        assertClosePct(comp._params1()._fcGain(), expectedFcGain, 10);
    };
    testArbitrary<Comp2>(setup, validate);
}

static void testF2Q(float qParam, float qcv, float expectedFcGain)
{
    auto setup = [qParam, qcv](Comp2& comp) {
        comp.params[Comp2::Q_PARAM].value = qParam;
        comp.inputs[Comp2::Q_INPUT].setVoltage(qcv, 0);
    };

    auto validate = [expectedFcGain](Comp2& comp) {
        assertClosePct(comp._params1()._qGain(), expectedFcGain, 10);
    };
    testArbitrary<Comp2>(setup, validate);
}


// All of these "expected" values are just harvested known goods.
// but this will let us test we break anything when we make it poly/simd
static void testF2Fc()
{
    testF2Fc(0, 0, .00058);
    testF2Fc(0, -10, .00058);
    testF2Fc(10, 0, .6);           // hmm... we are losing the top of the range - should scale it down below .5
    testF2Fc(10, 10, .6);
}

static void testF2Q()
{
    testF2Q(0, 0, 2);
    testF2Q(0, -10, 2);
    testF2Q(1, 0, .92);
    testF2Q(10, 0, .0099);
    testF2Q(10, 10, .0099);
}

void testFilterComposites()
{
    testF2Fc();
    testF2Q();
}

#endif