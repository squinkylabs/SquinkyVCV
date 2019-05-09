
#include "asserts.h"
#include "Filt.h"
#include "LadderFilter.h"
#include "TestComposite.h"


static void testLadderZero()
{
    LadderFilter<float> f;
    f.setNormalizedFc(.01f);
    for (int i = 0; i < 20; ++i) {
        f.run(0);
        float x = f.getOutput();
        assertEQ(x, 0);
    }
}

static void testLadderNotZero()
{
    LadderFilter<float> f;
    f.setNormalizedFc(.1f);
    for (int i = 0; i < 20; ++i) {
        f.run(1);
        float x = f.getOutput();
        assertGT(x, 0);
    }
}

static void testFilt()
{
    Filt<TestComposite> f;

    assert(!f.getVoicingNames().empty());
    assert(!f.getTypeNames().empty());

    assertEQ(f.getVoicingNames().size(), (int)LadderFilter<float>::Voicing::NUM_VOICINGS);
    assertEQ(f.getTypeNames().size(), (int) LadderFilter<float>::Types::NUM_TYPES);

}

static void testFilt2()
{
    Filt<TestComposite> f;
    auto x = f.getTypeNames();
    for (auto t : x) {
        assert(!t.empty());
    }

    auto y = f.getVoicingNames();
    for (auto t : y) {
        assert(!t.empty());
    }
}

void testLadder()
{
    testLadderZero();
    testLadderNotZero();
    testFilt();
    testFilt2();
}