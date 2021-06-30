
#include "asserts.h"

#include "CompiledInstrument.h"
#include "SamplerErrorContext.h"
#include "SInstrument.h"
#include "SParse.h"

   
/**
 * @param patch is an entire patch, regions marked with sample=bad should be pruned,
 *          regions with sample=good should remain
 */
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

/**
 *  @param goodValues are a list of opcodes that should "win" the prune
 * @param badVAlues are a list of opcodes that should "lose" the prune
 * 
 * tests will be run in both directions
 */
static void testSub2(const char* goodValues, const char* badValues) {
    std::string patch("<region>");
    patch += badValues;
    patch += " sample=bad\n";
    patch += "<region>";
    patch += goodValues;
    patch += " sample=good\n";
    testSub(patch.c_str());

    patch = "<region>";
    patch += goodValues;
    patch += " sample=good\n";
    patch += "<region>";
    patch += badValues;
    patch += " sample=bad\n";
    testSub(patch.c_str());
}

/**
 * Runs tests with both regsion orders, always expexts the region places first to win
 */
static void testTie(const char* firstValues, const char* secondValues) {
    std::string patch = "<region>";
    patch += firstValues;
    patch += " sample=good\n";
    patch += "<region>";
    patch += secondValues;
    patch += " sample=bad\n";
    testSub(patch.c_str());

    patch = "<region>";
    patch += secondValues;
    patch += " sample=good\n";
    patch += "<region>";
    patch += firstValues;
    patch += " sample=bad\n";
    testSub(patch.c_str());
}

static void test0() {
    testSub("<region>key=c3  sample=good");
}

static void testFirstWins() {
    testSub("<region>key=c3  sample=good <region>key=c3 sample=bad");
}

static void testNarrowPitchWins() {
    testSub("<region>lokey=c3 hikey=c4  sample=bad <region>key=c3 sample=good");

    // TODO: this should pass! bug? REGPRUNE
    // quick look it seems like we try to delete a region, but it doesn't succeed??
    //testSub2("key=c3", "lokey=c3 hikey=c4");
}

static void testReleaseSamples() {
    testSub2("key=c3", "key=c3 trigger=release");
}

static void testDamperPedal() {
    testSub("<region>key=c3 locc64=0 sample=good <region>key=c3 sample=bad");

    testTie("key=c3 locc64=0", "key=c3");
    testSub2("key=c3", "key=c3  locc64=1");

}

static void testDoesntAdjustPitch() {
    testSub("<region>lokey=c3 hikey=c4  sample=good <region>lokey=c4 hikey=c5 sample=bad");

}

static void testNegativeKey() {
    testSub2("key=3", "lokey=-1 hikey=-1");
    testSub2("key=3", "lokey=1 hikey=-1");
    testSub2("key=3", "lokey=-1 hikey=1");

    testTie("lokey=1 hikey=1", "key=1");
}

void testRegionPrune() {
    test0();
    testFirstWins();
    testNarrowPitchWins();
    testReleaseSamples();
    testDamperPedal();
    testNegativeKey();
}