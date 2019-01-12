
#include "LFN.h"
#include "asserts.h"
#include "FunVCOComposite.h"
#include "TestComposite.h"


template <class Comp>
inline static void test()
{
    std::shared_ptr<IComposite> comp(new Comp());

    assertGT(comp->getNumParams(), 0);
    for (int i = 0; i < comp->getNumParams(); ++i)
    {
        auto config = comp->getParam(i);
        assertLT(config.min, config.max);
        assertLE(config.def, config.max);
        assertGE(config.def, config.min);
    }
}

void testIComposite()
{
    test<FunVCOComposite<TestComposite>>();
    test<LFN<TestComposite>>();
}