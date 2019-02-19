
#include "CHB.h"
#include "ColoredNoise.h"
#include "EV3.h"
#include "FunVCOComposite.h"
#include "LFN.h"
#include "Gray.h"
#include "Seq.h"
#include "Shaper.h"
#include "Super.h"
#include "TestComposite.h"
#include "Tremolo.h"
#include "VocalAnimator.h"
#include "VocalFilter.h"

#include "asserts.h"
#include <set>


template <class Comp>
inline static void test()
{
    std::shared_ptr<IComposite> comp = Comp::getDescription();

    std::set<std::string> names;
    assertEQ(comp->getNumParams(), Comp::NUM_PARAMS);
    assertGT(comp->getNumParams(), 0);
    for (int i = 0; i < comp->getNumParams(); ++i)
    {
        auto config = comp->getParam(i);
        assertLT(config.min, config.max);
        assertLE(config.def, config.max);
        assertGE(config.def, config.min);

        assert(config.name);
        std::string name = config.name;
        assert(!name.empty());

        // make sure they are unique
        assert(names.find(name) == names.end());
        names.insert(name);
    }
}

void testIComposite()
{
    test<FunVCOComposite<TestComposite>>();
    test<LFN<TestComposite>>();
    test<VocalFilter<TestComposite>>();
    test<Shaper<TestComposite>>();
    test<CHB<TestComposite>>();
    test<Gray<TestComposite>>();
    test<Seq<TestComposite>>();
    test<VocalAnimator<TestComposite>>();
    test<Tremolo<TestComposite>>();
    test<Super<TestComposite>>();
    test<ColoredNoise<TestComposite>>();
    test<EV3<TestComposite>>();

    
}