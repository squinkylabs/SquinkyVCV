
#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "SParse.h"
#include "SamplerErrorContext.h"
#include "asserts.h"

static CompiledInstrumentPtr makeTest(const std::string& data) {

    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    assert(cinst);
    if (!errc.unrecognizedOpcodes.empty()) {
        errc.dump();
        assert(false);
    }

    return cinst;

}

// verify that region overrides others
static void testRegionAmpeg() {
    const char* data = R"foo(<global>ampeg_release=1
        <group>ampeg_release=2
        <region>ampeg_release=3
         )foo";

    auto inst = makeTest(data);

    std::vector<CompiledRegionPtr> regions;
    inst->_pool()._getAllRegions(regions);
    assertEQ(regions.size(), 1);
    CompiledRegionPtr creg = regions[0];
    assertEQ(creg->ampeg_release, 3);
}

// verify that group overrides others
static void testGroupAmpeg() {
    const char* data = R"foo(<global>ampeg_release=1
        <group>ampeg_release=2
        <region>
         )foo";

    auto inst = makeTest(data);

    std::vector<CompiledRegionPtr> regions;
    inst->_pool()._getAllRegions(regions);
    assertEQ(regions.size(), 1);
    CompiledRegionPtr creg = regions[0];
    assertEQ(creg->ampeg_release, 2);
}

// verify that global works
static void testGlobalAmpeg() {
    const char* data = R"foo(<global>ampeg_release=1
        <group>
        <region>
         )foo";

    auto inst = makeTest(data);

    std::vector<CompiledRegionPtr> regions;
    inst->_pool()._getAllRegions(regions);
    assertEQ(regions.size(), 1);
    CompiledRegionPtr creg = regions[0];
    assertEQ(creg->ampeg_release, 1);
}

// tests 1 ms default of ampeg_release
static void testDefaultAmpeg() {
    const char* data = R"foo(<global>
        <group>
        <region>
         )foo";

    auto inst = makeTest(data);

    std::vector<CompiledRegionPtr> regions;
    inst->_pool()._getAllRegions(regions);
    assertEQ(regions.size(), 1);
    CompiledRegionPtr creg = regions[0];
    assertClosePct(creg->ampeg_release, .001f, 1);
}

// test we prune release group
static void testRemoveRelease() {
    SQINFO("----- testRemoveRelease");
    const char* data = R"foo(<global>
        <region>sample=b key=1
        <group>trigger=release
        <region>sample=a key=2
         )foo";
    auto inst = makeTest(data);
    inst->_dump(0);
    std::vector<CompiledRegionPtr> regions;
    inst->_pool()._getAllRegions(regions);
    assertEQ(regions.size(), 1);
    CompiledRegionPtr creg = regions[0];
    assertEQ(creg->sampleFile, "b");
}

// test we prune release regions
static void testRemoveRelease2() {
    const char* data = R"foo(<global>
        <region>sample=b trigger=release key=1
        <region>sample=a key=2
         )foo";
    auto inst = makeTest(data);
    std::vector<CompiledRegionPtr> regions;
    inst->_pool()._getAllRegions(regions);
    assertEQ(regions.size(), 1);
    CompiledRegionPtr creg = regions[0];
    assertEQ(creg->sampleFile, "a");
}

static void testHicc() {
    const char* data = R"foo(<global>
        <region>sample=b hicc64=12 key=1
        <region>sample=a key=2 hicc64=20
         )foo";
    auto inst = makeTest(data);
    std::vector<CompiledRegionPtr> regions;
    inst->_pool()._getAllRegions(regions);
    assertEQ(regions.size(), 2);

    bool a = false;
    bool b = false;
    for (auto region : regions) {
        switch (region->lokey) {
            case 1:
                assertEQ(region->hicc64, 12);
                assertEQ(region->locc64, 0);
                a = true;
                break;
            case 2:
                assertEQ(region->hicc64, 20);
                assertEQ(region->locc64, 0);
                b = true;
                break;
            default:
                assert(false);
        }
    }
    assert(a && b);
}

static void testRemoveDamper() {
    const char* data = R"foo(<global>
        <region>sample=b hicc64=12 key=1
        <region>sample=a key=2
        <region>sample=c key=3 locc64=64
         )foo";
    auto inst = makeTest(data);
    std::vector<CompiledRegionPtr> regions;
    inst->_pool()._getAllRegions(regions);
    assertEQ(regions.size(), 2);
    // CompiledRegionPtr creg0 = regions[0];
    // CompiledRegionPtr creg1 = regions[1];
    //  assertEQ(creg0->sampleFile, "a");
    //  assertEQ(creg1->sampleFile, "c");
    //   assert(false);
    for (auto region : regions) {
        switch (region->lokey) {
            case 1:
                assert(region->locc64 == 0);
                assert(region->hicc64 == 12);
                break;
            case 2:
                assert(region->locc64 == 0);
                assert(region->hicc64 == 127);
                break;
            default:
                assert(false);
        }
    }
}

#if 0
static void  testDemoVel() {
    for (int v = 127; v> 0; v-=1) {
        int midiVelocity = v;
        float velTrack = 1.f;
        float g = CompiledInstrument::velToGain1(midiVelocity, velTrack);
        float g2 = CompiledInstrument::velToGain2(midiVelocity, velTrack);
        printf("v=%d, g1=%f g2=%f\n", midiVelocity, g, g2);
    }
    fflush(stdout);
    assert(false);
}
#endif

static void testVel() {
    // full scale
    assertClose(CompiledInstrument::velToGain(127, 100), 1, .05);
    assertClose(CompiledInstrument::velToGain(1, 100), 0, .05);
    assertClose(CompiledInstrument::velToGain(64, 100), 0.253953, .05);

    // no scale
    assertClose(CompiledInstrument::velToGain(127, 0), 1, .05);
    assertClose(CompiledInstrument::velToGain(1, 0), 1, .05);
    assertClose(CompiledInstrument::velToGain(64, 0), 1, .05);
}

void testx6() {
    testRegionAmpeg();
    testDefaultAmpeg();
    testGroupAmpeg();
    testGlobalAmpeg();
    testRemoveRelease();
    testRemoveRelease2();
    testHicc();
    testRemoveDamper();
    //   testDemoVel();
    testVel();
}