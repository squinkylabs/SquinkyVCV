

#include "CompiledInstrument.h"
#include "SParse.h"
#include "SamplerErrorContext.h"
#include "SInstrument.h"

#include "asserts.h"

static CompiledInstrumentPtr compile_it(const std::string& s) {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::go(s, inst);
   // auto err = SParse::goFile(FilePath(patch), inst);
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    assert(errc.empty());
    return cinst;
}

static void testGlobalParents() {
    const char* data = R"foo(
        <global>ampeg_release=2
        <region>key=31
         )foo";
    auto ci = compile_it(data);
    auto pool = ci->_pool();
    assertEQ(pool.size(), 1);

    std::vector<CompiledRegionPtr> x;
    pool._getAllRegions(x);
    auto region = x[0];
    auto ar = region->ampeg_release;
    assertEQ(ar, 2);
}

static void testMasterParents() {
    const char* data = R"foo(
        <master>ampeg_release=3
        <region>key=31
         )foo";
    auto ci = compile_it(data);
    auto pool = ci->_pool();
    assertEQ(pool.size(), 1);

    std::vector<CompiledRegionPtr> x;
    pool._getAllRegions(x);
    auto region = x[0];
    auto ar = region->ampeg_release;
    assertEQ(ar, 3);
}

static void testMideDoesntParent() {
    const char* data = R"foo(
        <midi>ampeg_release=2
        <region>key=31
         )foo";
    auto ci = compile_it(data);
    auto pool = ci->_pool();
    assertEQ(pool.size(), 1);

    std::vector<CompiledRegionPtr> x;
    pool._getAllRegions(x);
    auto region = x[0];
    auto ar = region->ampeg_release;
    assertEQ(ar, .03f);
}

void testx7() {
    testGlobalParents();
    testMasterParents();
    testMideDoesntParent();
}