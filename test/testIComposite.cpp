#include "TestComposite.h"
#include "Blank.h"
#include "ChaosKitty.h"
#include "CHB.h"
#include "ColoredNoise.h"

#include "FrequencyShifter.h"
#include "FunVCOComposite.h"
#include "LFN.h"
#include "LFNB.h"
#include "Filt.h"
#include "DrumTrigger.h"
#include "Gray.h"
#include "Mix8.h"
#include "Mix4.h"
#include "MixM.h"
#include "MixStereo.h"
#include "Seq.h"
#include "Seq4.h"
#include "Shaper.h"
#include "Slew4.h"
#include "Super.h"
#include "TestComposite.h"
#include "Tremolo.h"
#include "VocalAnimator.h"
#include "VocalFilter.h"
#include "daveguide.h"
#include "DividerX.h"
#include "F2_Poly.h"
#include "Samp.h"
#include "GMR.h"


#include "WVCO.h"
#include "Basic.h"
#include "Compressor.h"
#include "Compressor2.h"

#include "Sub.h"

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
    test<Seq4<TestComposite>>();
    test<VocalAnimator<TestComposite>>();
    test<Tremolo<TestComposite>>();
    test<Super<TestComposite>>();
    test<ColoredNoise<TestComposite>>();
//    test<EV3<TestComposite>>();
    test<FrequencyShifter<TestComposite>>();
    test<VocalAnimator<TestComposite>>();
    test<Blank<TestComposite>>();
    test<Slew4<TestComposite>>();
    test<Mix8<TestComposite>>();
    test<Mix4<TestComposite>>();
    test<MixStereo<TestComposite>>();
    test<MixM<TestComposite>>();
    test<LFNB<TestComposite>>();
    test<Filt<TestComposite>>();
    test<DrumTrigger<TestComposite>>();
    test<ChaosKitty<TestComposite>>();
    test<Daveguide<TestComposite>>();

    test<WVCO<TestComposite>>();
    test<Sub<TestComposite>>();
    test<Basic<TestComposite>>();
    test<Compressor<TestComposite>>();
    test<Compressor2<TestComposite>>();
    test<DividerX<TestComposite>>();
    test<F2_Poly<TestComposite>>();
    test<Samp<TestComposite>>();
    test<GMR<TestComposite>>();

}