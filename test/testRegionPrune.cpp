
#include "asserts.h"

#include "CompiledInstrument.h"
#include "SamplerErrorContext.h"
#include "SInstrument.h"
#include "SParse.h"

   
static void testSub(const char* patch) {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

   // auto err = SParse::goFile(FilePath(patch), inst);
    auto err = SParse::go(patch, inst);
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    assert(errc.empty());

    auto regionPool = cinst->_pool();
    assertEQ(regionPool.size(), 1);

    std::vector<CompiledRegionPtr> regions;
    regionPool._getAllRegions(regions);
    assertEQ(regions.size(), 1);

    CompiledRegionPtr region = regions[0];
    auto fileName = region->sampleFile.toString();
    assertEQ(fileName, "good");
    

}

static void test0() {
    testSub("<region>key=c3  sample=good");
}

static void testFirstWins() {
    testSub("<region>key=c3  sample=good <region>key=c3 sample=bad");
}

static void testNarrowPitchWins() {
    testSub("<region>lokey=c3 hikey=c4  sample=bad <region>key=c3 sample=good");
}

static void testReleaseSamples() {
    testSub("<region>key=c3 trigger=release sample=bad <region>key=c3  sample=good");
}

static void testReleaseSamples2() {
    testSub("<region>key=c3 trigger=attack sample=good <region>key=c3  sample=bad");
}

static void testDamperPedal() {
    testSub("<region>key=c3  locc64=1 sample=bad <region>key=c3 sample=good");
}

static void testDamperPedal2() {
    testSub("<region>key=c3 locc64=0 sample=good <region>key=c3 sample=bad");
}
static void testDoesntAdjustPitch() {
    testSub("<region>lokey=c3 hikey=c4  sample=good <region>lokey=c4 hikey=c5 sample=bad");
}

 //testCompileGroupSub(R"foo(<group>trigger=attack)foo", false);

/* TODO:
 release samples pruned
 damper pedal pruned
 check for others
*/
void testRegionPrune() {
    test0();
    testFirstWins();
    testNarrowPitchWins();
    testReleaseSamples();
    testReleaseSamples2();
    testDamperPedal();
    
    testDamperPedal2();
    //testDoesntAdjustPitch();
}