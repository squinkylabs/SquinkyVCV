
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

void testx6() {
    testRegionAmpeg();
    testDefaultAmpeg();
    testGroupAmpeg();
    testGlobalAmpeg();
}