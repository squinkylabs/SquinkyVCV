
#ifndef _MSC_VER 

#include "F2.h"
#include "TestComposite.h"

#include "tutil.h"
#include "asserts.h"

template <class T>
inline void testArbitrary( std::function<void(T&)> setup, std::function<void(T&)> validate)
{
    T comp;
    initComposite(comp);

    setup(comp);
    TestComposite::ProcessArgs args =  {44100, 1/44100}; 
    for (int i = 0; i < 40; ++i) {
        comp.process(args);
    }
    validate(comp);
}

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

// fcGain
static void testF2Fc()
{
    testF2Fc(0, 0, .00058);
    testF2Fc(10, 0, .6);           // hmm... we are losing the top of the range - should scale it down below .5
    testF2Fc(10, 10, .6);
}

void testFilterComposites()
{
    testF2Fc();
}

#endif