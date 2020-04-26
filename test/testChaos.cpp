
#include "ChaosKitty.h"
#include "TestComposite.h"
#include "asserts.h"

using Comp = ChaosKitty<TestComposite>;

void assertG(float g) {
    assert(g > 3.5);
    assert(g < 4);
}


static void testLabels()
{
    
    auto l = Comp::typeLabels();
    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    auto type = icomp->getParam(Comp::TYPE_PARAM);
    assertEQ(type.min, 0);
    assertEQ(type.max, l.size()-1);
}

void testChaos()
{
    testLabels();
}