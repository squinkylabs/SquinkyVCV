
#include <asserts.h>

#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "SLex.h"
#include "SParse.h"
#include "SqLog.h"
#include "SamplerErrorContext.h"
//#include "VelSwitch.h"

extern void testPlayInfoTinnyPiano();
extern void testPlayInfoSmallPiano();

#if 0
static void testVelSwitch1() {
    VelSwitch v(1234);

    SHeading h;
    SRegionPtr sr = std::make_shared<SRegion>(1234, h);
    SGroupPtr gp = std::make_shared<SGroup>(4567);
    gp->regions.push_back(sr);

    gp->compiledValues = SamplerSchema::compile(gp->values);
    sr->compiledValues = SamplerSchema::compile(sr->values);
    CompiledRegionPtr r0 = std::make_shared<CompiledRegion>(sr, nullptr, gp);

    // Need to make some "test" sample playback ptrs. just need to be able to recognize them later
    ISamplerPlaybackPtr p0 = std::make_shared<SimpleVoicePlayer>(r0, 0, 100);
    ISamplerPlaybackPtr p1 = std::make_shared<SimpleVoicePlayer>(r0, 0, 101);
    ISamplerPlaybackPtr p2 = std::make_shared<SimpleVoicePlayer>(r0, 0, 102);
    ISamplerPlaybackPtr p3 = std::make_shared<SimpleVoicePlayer>(r0, 0, 103);

    v.addVelocityRange(1, p0);  // index 1 starts at vel 1 (0 illegal
    v.addVelocityRange(2, p1);
    v.addVelocityRange(99, p2);
    v.addVelocityRange(100, p3);

    VoicePlayInfo info;
    ISamplerPlaybackPtr test = v.mapVelToPlayer(1);
    VoicePlayParameter params;
    params.midiPitch = 0;
    params.midiVelocity = 1;

  
    test->play(info, params, nullptr, 0);
    assertEQ(info.sampleIndex, 100);

    test = v.mapVelToPlayer(2);
    test->play(info, params, nullptr, 0);
    assertEQ(info.sampleIndex, 101);

    test = v.mapVelToPlayer(99);
    test->play(info, params, nullptr, 0);
    assertEQ(info.sampleIndex, 102);

    test = v.mapVelToPlayer(100);
    test->play(info, params, nullptr, 0);
    assertEQ(info.sampleIndex, 103);

    test = v.mapVelToPlayer(101);
    test->play(info, params, nullptr, 0);
    assertEQ(info.sampleIndex, 103);
}
#endif


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
        <region>
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
    static char* patch = R"foo(
        <group> sw_last=11 sw_label=key switch label 11
        sw_lokey=5 sw_hikey=15
        <region> key=50
       <group> sw_last=10 sw_label=key switch label
        sw_lokey=5 sw_hikey=15
        <region> key=50
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


// no selection
    VoicePlayInfo info;
    VoicePlayParameter params;
    info.sampleIndex = 0;
    params.midiPitch = 50;
    params.midiVelocity = 60;
    cinst->play(info, params, nullptr, 0);

    assert(!info.valid);
    cinst->play(info, params, nullptr, 0);
    assert(!info.valid);

// keyswitch
    params.midiPitch = 11;
    cinst->play(info, params, nullptr, 0);
    assert(!info.valid);

// other one
    params.midiPitch = 50;
    cinst->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.sampleIndex, 2);
}

static void testKeyswitch2() {
    static char* patch = R"foo(
        <group> sw_last=11 sw_label=key switch label 11
        sw_lokey=5 sw_hikey=15
        sw_default=10
        <region> key=50
       <group> sw_last=10 sw_label=key switch label
        sw_lokey=5 sw_hikey=15
        sw_default=10
        <region> key=50
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


// default keyswitch
    VoicePlayInfo info;
    VoicePlayParameter params;
    info.sampleIndex = 0;
    assert(!info.valid);
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
    assertEQ(info.sampleIndex, 2);
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

    testKeyswitch();
     testKeyswitch15();
    testKeyswitch2();

    assert(parseCount == 0);
    assert(compileCount == 0);
}