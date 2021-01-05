
#include <asserts.h>

#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "SParse.h"
#include "VelSwitch.h"

extern void testPlayInfoTinnyPiano();
extern void testPlayInfoSmallPiano();

static void testVelSwitch1() {
    VelSwitch v(1234);

    SRegionPtr sr = std::make_shared<SRegion>(1234);
    SGroupPtr gp = std::make_shared<SGroup>();
    gp->regions.push_back(sr);

    gp->compiledValues = SamplerSchema::compile(gp->values);
    sr->compiledValues = SamplerSchema::compile(sr->values);
    CompiledRegionPtr r0 = std::make_shared<CompiledRegion>(sr, nullptr, gp);

    // Need to make some "test" sample playback ptrs. just need to be able to recognize them later
    ISamplerPlaybackPtr p0 = std::make_shared<SimpleVoicePlayer>(r0, 100, 0);
    ISamplerPlaybackPtr p1 = std::make_shared<SimpleVoicePlayer>(r0, 101, 0);
    ISamplerPlaybackPtr p2 = std::make_shared<SimpleVoicePlayer>(r0, 102, 0);
    ISamplerPlaybackPtr p3 = std::make_shared<SimpleVoicePlayer>(r0, 103, 0);

    v.addVelocityRange(1, p0);  // index 1 starts at vel 1 (0 illegal
    v.addVelocityRange(2, p1);
    v.addVelocityRange(99, p2);
    v.addVelocityRange(100, p3);

    VoicePlayInfo info;
    ISamplerPlaybackPtr test = v.mapVelToPlayer(1);
    test->play(info, 0, 1);
    assertEQ(info.sampleIndex, 100);

    test = v.mapVelToPlayer(2);
    test->play(info, 0, 1);
    assertEQ(info.sampleIndex, 101);

    test = v.mapVelToPlayer(99);
    test->play(info, 0, 1);
    assertEQ(info.sampleIndex, 102);

    test = v.mapVelToPlayer(100);
    test->play(info, 0, 1);
    assertEQ(info.sampleIndex, 103);

    test = v.mapVelToPlayer(101);
    test->play(info, 0, 1);
    assertEQ(info.sampleIndex, 103);
}


// Note that making a region out of the context of an insturment is now quite involved.
// We may need a test halper for this if we plan on doing it much.

static CompiledRegionPtr makeTestRegion(SGroupPtr gp, bool usePitch, const std::string& minVal, const std::string& maxVal) {
    SRegionPtr sr = std::make_shared<SRegion>(1234);
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
    sr->compiledValues = SamplerSchema::compile(sr->values);
    CompiledRegionPtr r0 = std::make_shared<CompiledRegion>(sr, nullptr, gp);
    return r0;
}

static void testOverlapSub(bool testPitch, int mina, int maxa, int minb, int maxb, bool shouldOverlap) {
    assert(mina <= maxa);
    SGroupPtr gp = std::make_shared<SGroup>();
    gp->compiledValues = SamplerSchema::compile(gp->values);
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

static void testSmallPianoVelswitch() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::goFile(smallPiano, inst);
    assert(err.empty());

    CompiledInstrumentPtr cinst = CompiledInstrument::make(inst);
    VoicePlayInfo info;
    cinst->play(info, 60, 60);
    assert(info.valid);

    //  {1, 23, 44, 65, 80, 107});
    cinst->play(info, 64, 1);
    const int si1 = info.sampleIndex;

    cinst->play(info, 64, 22);
    const int si22 = info.sampleIndex;

    cinst->play(info, 64, 23);
    const int si23 = info.sampleIndex;

    cinst->play(info, 64, 30);
    const int si30 = info.sampleIndex;

    cinst->play(info, 64, 43);
    const int si43 = info.sampleIndex;

    cinst->play(info, 64, 44);
    const int si44 = info.sampleIndex;

    cinst->play(info, 64, 107);
    const int si107 = info.sampleIndex;

    cinst->play(info, 64, 127);
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

    CompiledInstrumentPtr cinst = CompiledInstrument::make(inst);
    VoicePlayInfo info;
    cinst->_dump(0);
}

void testx3() {
    // work up to these
    assert(parseCount == 0);

    testVelSwitch1();
    testOverlap();

    testSmallPianoVelswitch();

    // Note: this tests are in testx2. Just moved here for logical
    // sequencing reasons.
    testPlayInfoTinnyPiano();
    testPlayInfoSmallPiano();
    testSnareBasic();

    assert(parseCount == 0);
    assert(compileCount == 0);
}