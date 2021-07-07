
#include <asserts.h>

#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "SLex.h"
#include "SParse.h"
#include "SamplerErrorContext.h"
#include "SqLog.h"
#include "WaveLoader.h"

extern void testPlayInfoTinnyPiano();
extern void testPlayInfoSmallPiano();

// Note that making a region out of the context of an insturment is now quite involved.
// We may need a test halper for this if we plan on doing it much.


static CompiledRegionPtr makeTestRegion(bool usePitch, const std::string& minVal, const std::string& maxVal) {
    //SHeading h;
   // SRegionPtr sr = std::make_shared<SRegion>(1234, h);
  //  gp->regions.push_back(sr);
    SHeadingPtr sr = std::make_shared<SHeading>(SHeading::Type::Region, 1234);

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

    CompiledRegionPtr r0 = std::make_shared<CompiledRegion>(45);
    r0->addRegionInfo(sr->compiledValues);

    //  //CompiledRegionPtr r0 = std::make_shared<CompiledRegion>(sr, nullptr, gp);
    return r0;
}


static void testOverlapSub(bool testPitch, int mina, int maxa, int minb, int maxb, bool shouldOverlap) {
    assert(mina <= maxa);

    SamplerErrorContext errc;
    auto regionA = makeTestRegion(testPitch, std::to_string(mina), std::to_string(maxa));
    auto regionB = makeTestRegion(testPitch, std::to_string(minb), std::to_string(maxb));
    bool overlap = testPitch ? regionA->overlapsPitch(*regionB) : regionA->overlapsVelocity(*regionB);
    assertEQ(overlap, shouldOverlap);
}

static void testRegionOverlap(bool testPitch) {
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

static void testRegionOverlap() {
    testRegionOverlap(true);
    testRegionOverlap(false);
}

static void testParitalOverlapSub(bool testPitch, int mina, int maxa, int minb, int maxb, float expectedOverlap, int expectedIntOverlap) {
    assert(mina <= maxa);
    assert(minb <= maxb);
  //  SGroupPtr gp = std::make_shared<SGroup>(1234);
    SamplerErrorContext errc;
  //  gp->compiledValues = SamplerSchema::compile(errc, gp->values);
    auto regionA = makeTestRegion(testPitch, std::to_string(mina), std::to_string(maxa));
    auto regionB = makeTestRegion(testPitch, std::to_string(minb), std::to_string(maxb));
    auto overlap = testPitch ? regionA->overlapPitchAmount(*regionB) : regionA->overlapVelocityAmount(*regionB);
   // assertEQ(overlap, shouldOverlap)
    assertClose(overlap.second, expectedOverlap, .01);
    assertEQ(overlap.first, expectedIntOverlap);
}

static void testRegionPartialOverlap(bool testPitch) {
    testParitalOverlapSub(testPitch, 10, 20, 30, 40, 0, 0);         // no overlap
    testParitalOverlapSub(testPitch, 1, 2, 2, 3, .5f, 1);           // one overlap, simple case
    testParitalOverlapSub(testPitch, 10, 20, 20, 30, .1f, 1);       // one overlap
    testParitalOverlapSub(testPitch, 10, 20, 10, 20, 1, 11);
    testParitalOverlapSub(testPitch, 10, 20, 15, 25, 6.f / 11.f, 6);
    testParitalOverlapSub(testPitch, 15, 25, 10, 20, 6.f / 11.f, 6);
}
static void testRegionPartialOverlap() {
    testRegionPartialOverlap(false);
    testRegionPartialOverlap(true); 
}

static char* smallPiano = R"foo(D:\samples\K18-Upright-Piano\K18-Upright-Piano.sfz)foo";
static char* snare = R"foo(D:\samples\SalamanderDrumkit\snare.sfz)foo";
static char* allSal = R"foo(D:\samples\SalamanderDrumkit\all.sfz)foo";

static void testPlaySmallPianoVelswitch() {
     SQWARN("\n----  testSmallPianoVelswitch\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::goFile(FilePath(smallPiano), inst);
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    SQWARN("----  fix this bug and put the test back\n");


    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 60;
    params.midiVelocity = 60;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);

    params.midiPitch = 64;
    params.midiVelocity = 1;
    cinst->play(info, params, nullptr, 44100);
    const int si1 = info.sampleIndex;

    params.midiVelocity = 22;
    cinst->play(info, params, nullptr, 44100);
    const int si22 = info.sampleIndex;

    params.midiVelocity = 23;
    cinst->play(info, params, nullptr, 44100);
    const int si23 = info.sampleIndex;

    params.midiVelocity = 30;
    cinst->play(info, params, nullptr, 44100);
    const int si30 = info.sampleIndex;

    params.midiVelocity = 43;
    cinst->play(info, params, nullptr, 44100);
    const int si43 = info.sampleIndex;

    params.midiVelocity = 44;
    cinst->play(info, params, nullptr, 44100);
    const int si44 = info.sampleIndex;

    params.midiVelocity = 107;
    cinst->play(info, params, nullptr, 44100);
    const int si107 = info.sampleIndex;

    params.midiVelocity = 127;
    cinst->play(info, params, nullptr, 44100);
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

static void testPlaySnareBasic() {
    printf("\n------- testSnareBasic\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::goFile(FilePath(snare), inst);
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    VoicePlayInfo info;
}

static void testPlayAllSal() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::goFile(FilePath(allSal), inst);
    if (!err.empty()) SQFATAL(err.c_str());
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    VoicePlayInfo info;
    // cinst->_dump(0);
}

static void testCompileKeswitch() {
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
static void testCompileKeswitchOverlap() {
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

static void testCompileKeyswitch() {
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
static void testPlayKeyswitch15() {
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
    cinst->play(info, params, nullptr, 44100);

    assert(!info.valid);
    cinst->play(info, params, nullptr, 44100);
    assert(!info.valid);

    // keyswitch with pitch 11
    params.midiPitch = 11;
    cinst->play(info, params, nullptr, 44100);
    assert(!info.valid);

    // now play 50 again, should play first region
    params.midiPitch = 50;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 1);

    // keyswitch with pitch 10
    params.midiPitch = 10;
    cinst->play(info, params, nullptr, 44100);
    assert(!info.valid);

    // now play 50 again, should play second region
    params.midiPitch = 50;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 2);
}

static void testPlayKeyswitch2() {
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
    cinst->play(info, params, nullptr, 44100);

    assert(info.valid);
    assertEQ(info.sampleIndex, 2);
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 2);

    // keyswitch
    params.midiPitch = 11;
    cinst->play(info, params, nullptr, 44100);
    assert(!info.valid);

    // other one
    params.midiPitch = 50;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 1);
}

static void testPlayOverlapVel() {
    SQINFO("---- testOverlapVel");
    static char* patch = R"foo(
    <group> // kick - snares on
    key=48
    volume=10
    <region> 
    sample=a
    lovel=0
    hivel=30
    <region>
    sample=b
    lovel=30
    hivel=60
    <region>
    sample=c
    lovel=60
    hivel=90
    <region>
    sample=d
    lovel=90
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
    const size_t c = regions.size();

    VoicePlayInfo info;
    VoicePlayParameter params;
    
    params.midiPitch= 48;
    params.midiVelocity = 1;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 1);

    // first region gets shrunk, so second region plays here
    params.midiVelocity = 30;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 2);  

    params.midiVelocity = 31;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 2);

    params.midiVelocity = 59;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 2);

    // second gets shrunk
    params.midiVelocity = 60;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 3);

    params.midiVelocity = 61;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 3);

    params.midiVelocity = 90;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 3);

    params.midiVelocity = 91;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 4);

    params.midiVelocity = 127;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 4);
}

static void testPlayOverlapPitch() {
    SQINFO("---- testOverlapVel");
    static char* patch = R"foo(
    <region> 
    sample=a
    lokey=0
    hikey=30
    <region>
    sample=b
    lokey=30
    hikey=60
    <region>
    sample=c
    lokey=60
    hikey=90
    <region>
    sample=d
    lokey=90
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
    const size_t c = regions.size();

    VoicePlayInfo info;
    VoicePlayParameter params;

    params.midiPitch = 1;
    params.midiVelocity = 64;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 1);

    // first region gets shrunk, so second region plays here
    params.midiPitch = 30;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 2);

    params.midiPitch = 31;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 2);

    params.midiPitch = 59;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 2);

    // second gets shrunk
    params.midiPitch = 60;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 3);

    params.midiPitch = 61;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 3);

    params.midiPitch = 90;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 3);

    params.midiPitch = 91;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 4);

    params.midiPitch = 127;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 4);
}

static void testPlayOverlapRestore() {
     static char* patch = R"foo(
    <region> 
    sample=a
    lokey=0
    hikey=127
    <region>
    sample=b
    lokey=30
    hikey=60
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
    const size_t c = regions.size();
    CompiledRegion* r = regions[0].get();

    VoicePlayInfo info;
    VoicePlayParameter params;

    // big region deleted, small one intact
    params.midiPitch = 1;
    params.midiVelocity = 64;
    cinst->play(info, params, nullptr, 44100);
    assert(!info.valid);

    params.midiPitch = 29;
    params.midiVelocity = 64;
    cinst->play(info, params, nullptr, 44100);
    assert(!info.valid);

    params.midiPitch = 30;
    params.midiVelocity = 64;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 1);

    params.midiPitch = 60;
    params.midiVelocity = 64;
    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.sampleIndex, 1);
  
    params.midiPitch = 61;
    params.midiVelocity = 64;
    cinst->play(info, params, nullptr, 44100);
    assert(!info.valid);

    params.midiPitch = 127;
    params.midiVelocity = 64;
    cinst->play(info, params, nullptr, 44100);
    assert(!info.valid);
}

static void testPlayLoop() {
    const char* data = (R"foo(
          <group>loop_mode=one_shot
          <region>sample=a offset=222 loop_start=91 loop_end=112
          )foo");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto cinst = CompiledInstrument::make(errc, inst);

    assertEQ(errc.unrecognizedOpcodes.size(), 0);

    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 12;
    params.midiVelocity = 60;

    cinst->play(info, params, nullptr, 44100);
    assert(info.valid);
    assertEQ(info.loopData.offset, 222);
    assertEQ(info.loopData.loop_start, 91);
    assertEQ(info.loopData.loop_end, 112);
    assertEQ((int)info.loopData.loop_mode, (int)SamplerSchema::DiscreteValue::ONE_SHOT);

}

#if 0
static void testPlayOsc() {
    SQINFO("-- testPlayOsc --");
    // for reference - normal pitch
    {
        const char* data = (R"foo(
          <region>sample=a
                key=60 
          )foo");
        SInstrumentPtr inst = std::make_shared<SInstrument>();
        auto err = SParse::go(data, inst);

        SamplerErrorContext errc;
        auto cinst = CompiledInstrument::make(errc, inst);
        assertEQ(errc.unrecognizedOpcodes.size(), 0);

        WaveLoaderPtr w = std::make_shared<WaveLoader>();
        w->_setTestMode(WaveLoader::Tests::DCOneSec);

        VoicePlayInfo info;
        VoicePlayParameter params;
        params.midiPitch = 60;              // middle C
        params.midiVelocity = 60;

        cinst->play(info, params, w.get(), 44100);
        SQINFO("normal play, transv = %f", info.transposeV);
        assert(info.valid);
    }

    // now the test of loop
    {
        SQINFO("-- testPlayOsc (looped) --");
        const char* data = (R"foo(
          <region>sample=a oscillator=on
          )foo");
        SInstrumentPtr inst = std::make_shared<SInstrument>();
        auto err = SParse::go(data, inst);

        SamplerErrorContext errc;
        auto cinst = CompiledInstrument::make(errc, inst);
        assertEQ(errc.unrecognizedOpcodes.size(), 0);

        WaveLoaderPtr w = std::make_shared<WaveLoader>();
        w->_setTestMode(WaveLoader::Tests::Zero2048);

        VoicePlayInfo info;
        VoicePlayParameter params;
        params.midiPitch = 60;              // middle C
        params.midiVelocity = 60;

        cinst->play(info, params, w.get(), 44100);
        assert(info.valid);
        SQINFO("loop play, transv = %f", info.transposeV);
    }

    assert(false);

}
#endif

static void compileShouldFindMalformed(const char* input) {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::go(input, inst);
    if (!err.empty()) SQFATAL(err.c_str());
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    assert(cinst);
    if (!errc.empty()) {
        errc.dump();
    }
    assert(!errc.empty());
}

static void testCompileMalformedRelease() {
    compileShouldFindMalformed(R"foo(
        <region>ampeg_release=abcd
        )foo");
    compileShouldFindMalformed(R"foo(
        <region>ampeg_release=qb.3
        )foo");
}

static void testCompileMalformedKey() {
    compileShouldFindMalformed(R"foo(
        <region>key=abcd
        )foo");
    compileShouldFindMalformed(R"foo(
        <region>key=c#
        )foo");
    compileShouldFindMalformed(R"foo(
        <region>key=cn
        )foo");
    compileShouldFindMalformed(R"foo(
        <region>key=c.
        )foo");
    compileShouldFindMalformed(R"foo(
        <region>key=h3
        )foo");
}

void testx3() {
    testPlayAllSal();
    // work up to these
    assert(parseCount == 0);

    //  testVelSwitch1();
    testRegionOverlap();
    testRegionPartialOverlap();

    testPlaySmallPianoVelswitch();

    // Note: this tests are in testx2. Just moved here for logical
    // sequencing reasons.
    testPlayInfoTinnyPiano();
    testPlayInfoSmallPiano();
    testPlaySnareBasic();
    testPlayAllSal();

    testCompileKeswitch();
    testCompileKeswitchOverlap();
    testCompileKeyswitch();
    testPlayKeyswitch15();
    testPlayKeyswitch2();

    testCompileMalformedRelease();
    testCompileMalformedKey();

    testPlayOverlapVel();
    testPlayOverlapPitch();
    testPlayOverlapRestore();
    testPlayLoop();
    //testPlayOsc();

    assert(parseCount == 0);
    assert(compileCount == 0);
}