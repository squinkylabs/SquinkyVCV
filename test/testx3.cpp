
#include <asserts.h>
#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "SLex.h"
#include "SParse.h"
#include "SqLog.h"
#include "SamplerErrorContext.h"

extern void testPlayInfoTinnyPiano();
extern void testPlayInfoSmallPiano();

// Note that making a region out of the context of an insturment is now quite involved.
// We may need a test halper for this if we plan on doing it much.

static CompiledRegionPtr makeTestRegion(SGroupPtr gp, bool usePitch, const std::string& minVal, const std::string& maxVal) {
    SHeading h;
    SRegionPtr sr = std::make_shared<SRegion>(1234, h);
    gp->regions.push_back(sr);

    SKeyValuePairPtr kv;
    if (usePitch) {
        kv = std::make_shared<SKeyValuePair>("lokey", minVal);
        sr->values.push_back(kv);
        kv = std::make_shared<SKeyValuePair>("hikey", maxVal);
        sr->values.push_back(kv);
    } else {
        kv = std::make_shared<SKeyValuePair>("lovel", minVal);
        sr->values.push_back(kv);
        kv = std::make_shared<SKeyValuePair>("hivel", maxVal);
        sr->values.push_back(kv);
    }
    SamplerErrorContext errc;
    sr->compiledValues = SamplerSchema::compile(errc, sr->values);
    assert(errc.empty());
    CompiledRegionPtr r0 = std::make_shared<CompiledRegion>(sr, nullptr, gp);
    return r0;
}

static void testOverlapSub(bool testPitch, int mina, int maxa, int minb, int maxb, bool shouldOverlap) {
    assert(mina <= maxa);
    SGroupPtr gp = std::make_shared<SGroup>(1234);
    SamplerErrorContext errc;
    gp->compiledValues = SamplerSchema::compile(errc, gp->values);
    auto regionA = makeTestRegion(gp, testPitch, std::to_string(mina), std::to_string(maxa));
    auto regionB = makeTestRegion(gp, testPitch, std::to_string(minb), std::to_string(maxb));
    bool overlap = testPitch ? regionA->overlapsPitch(*regionB) : regionA->overlapsVelocity(*regionB);
    assertEQ(overlap, shouldOverlap);
}

static void testOverlap(bool testPitch) {
    // negative tests
    testOverlapSub(testPitch, 10, 20, 30, 40, false);
    testOverlapSub(testPitch, 50, 60, 30, 40, false);
    testOverlapSub(testPitch, 1, 1, 2, 2, false);
    testOverlapSub(testPitch, 1, 40, 41, 127, false);
    testOverlapSub(testPitch, 2, 2, 1, 1, false);
    testOverlapSub(testPitch, 41, 50, 1, 40, false);
    testOverlapSub(testPitch, 1, 1, 127, 127, false);
    testOverlapSub(testPitch, 127, 127, 1, 1, false);

    // positive
    testOverlapSub(testPitch, 10, 20, 15, 25, true);
    testOverlapSub(testPitch, 15, 25, 10, 20, true);
    testOverlapSub(testPitch, 12, 12, 12, 12, true);
    testOverlapSub(testPitch, 10, 50, 50, 60, true);
    testOverlapSub(testPitch, 50, 60, 10, 50, true);
    testOverlapSub(testPitch, 10, 50, 1, 100, true);

    // assert(false);
}

static void testOverlap() {
    testOverlap(true);
    testOverlap(false);
}

static char* smallPiano = R"foo(D:\samples\K18-Upright-Piano\K18-Upright-Piano.sfz)foo";
static char* snare = R"foo(D:\samples\SalamanderDrumkit\snare.sfz)foo";
static char* allSal = R"foo(D:\samples\SalamanderDrumkit\all.sfz)foo";

static void testSmallPianoVelswitch() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::goFile(smallPiano, inst);
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 60;
    params.midiVelocity = 60;
    cinst->play(info, params, nullptr, 0);
    assert(info.valid);


    params.midiPitch = 64;
    params.midiVelocity = 1;
    cinst->play(info, params, nullptr, 0);
    const int si1 = info.sampleIndex;

    params.midiVelocity = 22;
    cinst->play(info, params, nullptr, 0);
    const int si22 = info.sampleIndex;

    params.midiVelocity = 23;
    cinst->play(info, params, nullptr, 0);
    const int si23 = info.sampleIndex;

    params.midiVelocity = 30;
    cinst->play(info, params, nullptr, 0);
    const int si30 = info.sampleIndex;

    params.midiVelocity = 43;
    cinst->play(info, params, nullptr, 0);
    const int si43 = info.sampleIndex;

    params.midiVelocity = 44;
    cinst->play(info, params, nullptr, 0);
    const int si44 = info.sampleIndex;

    params.midiVelocity = 107;
    cinst->play(info, params, nullptr, 0);
    const int si107 = info.sampleIndex;

    params.midiVelocity = 127;
    cinst->play(info, params, nullptr, 0);
    const int si127 = info.sampleIndex;

    assertEQ(si1, si22);
    assertNE(si23, si22);

    assertEQ(si30, si23);
    assertEQ(si43, si30);
    assertNE(si44, si43);

    assertNE(si107, si44);
    assertEQ(si107, si127);

    assertNE(si1, si44);
}

static void testSnareBasic() {
    printf("\n------- testSnareBasic\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::goFile(snare, inst);
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    VoicePlayInfo info;
    // cinst->_dump(0);
}

static void testAllSal() {
    SQINFO("\n------- testAllSal");

    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::goFile(allSal, inst);
    if (!err.empty()) SQFATAL(err.c_str());
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    VoicePlayInfo info;
    // cinst->_dump(0);
}


static void testKeswitchCompiled() {
     SQINFO("\n------ testKeyswitchCompiled");
     static char* patch = R"foo(
       <group> sw_last=10 sw_label=key switch label
        sw_lokey=5 sw_hikey=15
        lokey=9
        hikey=11
        <region>
        <group>
        <region>key=100 sw_default=100;
        )foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(patch, inst);
    if (!err.empty()) SQFATAL(err.c_str());
    assert(err.empty());
    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    assert(cinst);
    if (!errc.empty()) {
        errc.dump();
    }
    const RegionPool& pool = cinst->_pool();
    std::vector<CompiledRegionPtr> regions;
    pool._getAllRegions(regions);
    assertEQ(regions.size(), 2);


    // region with switch is not enabled yet
    assertEQ(regions[0]->isKeyswitched(), false);
    assertEQ(regions[0]->sw_lolast, 10);
    assertEQ(regions[0]->sw_hilast, 10);
    assertEQ(regions[0]->sw_lokey, 5);
    assertEQ(regions[0]->sw_hikey, 15);
    assertEQ(regions[0]->sw_default, -1);
    assertEQ(regions[0]->sw_label, "key switch label");

    // second region doesn't have any ks, so it's on.
    assertEQ(regions[1]->isKeyswitched(), true);
    assertEQ(regions[1]->sw_lolast, -1);
    assertEQ(regions[1]->sw_hilast, -1);
    assertEQ(regions[1]->sw_default, 100);
}

// two regions at same pitch, but never on at the same time
static void testKeswitchCompiledOverlap() {
      SQINFO("\n------ testKeyswitchCompiled");
     static char* patch = R"foo(
       <group>
        lokey=9
        hikey=11
        sw_last=10
        <region>
        <group>
        <region>
        lokey=9
        hikey=11
        sw_last=11
        )foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(patch, inst);
    if (!err.empty()) SQFATAL(err.c_str());
    assert(err.empty());
    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    assert(cinst);
    if (!errc.empty()) {
        errc.dump();
    }
    const RegionPool& pool = cinst->_pool();
    std::vector<CompiledRegionPtr> regions;
    pool._getAllRegions(regions);
    assertEQ(regions.size(), 2);
}

static void testKeyswitch() {
    SQINFO("\n------ testKeyswitch");
#if 0 // this one made the lexer freak out. Let's investigage (later)
    static char* patch = R"foo(
        <group> sw_last=10 sw_label=key switch label
        sw_lokey=5 swhikey=15
        sw_default=10
        <region>
        z)foo";
#endif
    static char* patch = R"foo(
       <group> sw_last=10 sw_label=key switch label
        sw_lokey=5 sw_hikey=15
        sw_default=10
        <region> lokey=30 hikey=30
        )foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::go(patch, inst);
    if (!err.empty()) SQFATAL(err.c_str());
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    assert(cinst);
    if (!errc.empty()) {
        errc.dump();
    }
    assert(errc.empty());
}

// this one has not default
static void testKeyswitch15() {
      SQINFO("\n------ testKeyswitch15");
    static char* patch = R"foo(
        <group> sw_last=11 sw_label=key switch label 11
        sw_lokey=5 sw_hikey=15
        <region> key=50 sample=foo
       <group> sw_last=10 sw_label=key switch label
        sw_lokey=5 sw_hikey=15
        <region> key=50 sample=bar
        )foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::go(patch, inst);
    if (!err.empty()) SQFATAL(err.c_str());
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    assert(cinst);
    if (!errc.empty()) {
        errc.dump();
    }
    assert(errc.empty());


// no selection, won't play
    VoicePlayInfo info;
    VoicePlayParameter params;
    info.sampleIndex = 0;
    params.midiPitch = 50;
    params.midiVelocity = 60;
    cinst->play(info, params, nullptr, 0);

    assert(!info.valid);
    cinst->play(info, params, nullptr, 0);
    assert(!info.valid);

// keyswitch with pitch 11
    params.midiPitch = 11;
    cinst->play(info, params, nullptr, 0);
    assert(!info.valid);

// now play 50 again, should play first region
    params.midiPitch = 50;
    cinst->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.sampleIndex, 1);

// keyswitch with pitch 10
    params.midiPitch = 10;
    cinst->play(info, params, nullptr, 0);
    assert(!info.valid);

// now play 50 again, should play second region
    params.midiPitch = 50;
    cinst->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.sampleIndex, 2);


}

static void testKeyswitch2() {
      SQINFO("\n------ testKeyswitch 2");
    static char* patch = R"foo(
        <group> sw_last=11 sw_label=key switch label 11
        sw_lokey=5 sw_hikey=15
        sw_default=10
        <region> key=50 sample=foo
       <group> sw_last=10 sw_label=key switch label
        sw_lokey=5 sw_hikey=15
        sw_default=10
        <region> key=50 sample=bar
        )foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::go(patch, inst);
    if (!err.empty()) SQFATAL(err.c_str());
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    assert(cinst);
    if (!errc.empty()) {
        errc.dump();
    }
    assert(errc.empty());
    std::vector<CompiledRegionPtr> regions;
    cinst->_pool()._getAllRegions(regions);
    assertEQ(regions.size(), 2);
    assertEQ(regions[0]->sw_default, 10);
    assertEQ(regions[1]->sw_default, 10);


// default keyswitch
    VoicePlayInfo info;
    VoicePlayParameter params;
    info.sampleIndex = 0;
    assert(!info.valid);

    // play pitch 50
    params.midiPitch = 50;
    params.midiVelocity = 60;
    cinst->play(info, params, nullptr, 0);

    assert(info.valid);
    assertEQ(info.sampleIndex, 2);
    cinst->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.sampleIndex, 2);

// keyswitch
    params.midiPitch = 11;
    cinst->play(info, params, nullptr, 0);
    assert(!info.valid);

// other one
    params.midiPitch = 50;
    cinst->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.sampleIndex, 1);
}

void testx3() {

  

    testAllSal();
    // work up to these
    assert(parseCount == 0);

  //  testVelSwitch1();
    testOverlap();

    testSmallPianoVelswitch();

    // Note: this tests are in testx2. Just moved here for logical
    // sequencing reasons.
    testPlayInfoTinnyPiano();
    testPlayInfoSmallPiano();
    testSnareBasic();
    testAllSal();


    testKeswitchCompiled();
    testKeswitchCompiledOverlap();
    testKeyswitch();
    testKeyswitch15();
    testKeyswitch2();


    assert(parseCount == 0);
    assert(compileCount == 0);
}