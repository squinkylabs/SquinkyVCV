
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

void testx6() {
    testRegionAmpeg();
}