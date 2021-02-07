#include <set>

#include "CompiledInstrument.h"
#include "CompiledRegion.h"
#include "CubicInterpolator.h"
#include "FilePath.h"
#include "SInstrument.h"
#include "Sampler4vx.h"
#include "SamplerSchema.h"
#include "SqLog.h"
#include "Streamer.h"
#include "WaveLoader.h"
#include "asserts.h"
#include "samplerTests.h"


static const char* tinnyPiano = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\UprightPianoKW-small-20190703.sfz)foo";
const char* tinnyPianoRoot = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\)foo";
static const char* smallPiano = R"foo(D:\samples\K18-Upright-Piano\K18-Upright-Piano.sfz)foo";

static void testWaveLoader0() {
    WaveLoader w;
    w.addNextSample("fake file name");
    const bool b = w.load();
    assert(!b);
}

static void testWaveLoader1() {
    WaveLoader w;
    w.addNextSample("D:\\samples\\UprightPianoKW-small-SFZ-20190703\\samples\\A3vH.wav");
    const bool b = w.load();
    assert(b);
    auto x = w.getInfo(1);
    assert(x->valid);
    x = w.getInfo(0);
    assert(!x);
}

static void testWaveLoaderNot44() {
    WaveLoader w;
    w.addNextSample("D:\\samples\\K18-Upright-Piano\\K18\\A0.f.wav");

    const bool b = w.load();
    assert(b);

    auto x = w.getInfo(1);
    assert(x->valid);
    assertEQ(x->sampleRate, 48000);  // we keep the input sample rate
    assertEQ(x->numChannels, 1);     // but we convert to mono
}

static void testWaveLoaderNotMono() {
    assert(false);
}

static void testPlayInfo() {
    VoicePlayInfo info;
    assertEQ(info.valid, false);
}

static void testPlayInfo(const char* patch, const std::vector<int>& velRanges) {
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    auto err = SParse::goFile(patch, inst);
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    // assert(errc.empty());
    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 60;
    params.midiVelocity = 60;
    cinst->play(info, params, nullptr, 0);
    assert(info.valid);
    int minSampleIndex = 200;
    int maxSampleIndex = -200;
    for (int pitch = 21; pitch <= 108; ++pitch) {
        for (auto vel : velRanges) {
            info.valid = false;
            params.midiPitch = pitch;
            params.midiVelocity = vel;
            cinst->play(info, params, nullptr, 0);
            assert(info.valid);
            assert(info.canPlay());
            minSampleIndex = std::min(minSampleIndex, info.sampleIndex);
            maxSampleIndex = std::max(maxSampleIndex, info.sampleIndex);
        }
    }

    // VoicePlayParameter params;
    params.midiPitch = 20;
    params.midiVelocity = 60;
    cinst->play(info, params, nullptr, 0);
    assert(!info.valid);

    params.midiPitch = 109;
    cinst->play(info, params, nullptr, 0);
    assert(!info.valid);

    assert(minSampleIndex == 1);
    assert(maxSampleIndex > 4);
}

void testPlayInfoTinnyPiano() {
    testPlayInfo(tinnyPiano, {64});
}

void testPlayInfoSmallPiano() {
    printf("\n----- testPlayInfoSmallPiano\n");
    testPlayInfo(smallPiano, {1, 23, 44, 65, 80, 107});
}

static void testLoadWavesPiano() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    //  const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\UprightPianoKW-small-20190703.sfz)foo";
    auto err = SParse::goFile(tinnyPiano, inst);
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    WaveLoaderPtr loader = std::make_shared<WaveLoader>();

    // const char* pRoot = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\)foo";
    cinst->setWaves(loader, tinnyPianoRoot);
    loader->load();
    // assert(false);
}

static void testStream() {
    Streamer s;
    assert(!s.canPlay(0));
    assert(!s.canPlay(1));
    assert(!s.canPlay(2));
    assert(!s.canPlay(3));
    s.step();

    float x[6] = {0};
    s.setSample(0, x, 6);
    assert(s.canPlay(0));
    assert(!s.canPlay(1));
}

static void testStreamEnd() {
    Streamer s;
    const int channel = 2;
    assert(!s.canPlay(channel));

    float x[6] = {0};
    s.setSample(channel, x, 6);
    assert(s.canPlay(channel));
    for (int i = 0; i < 6; ++i) {
        s.step();
    }
    assert(!s.canPlay(channel));
}

static void testStreamValues() {
    Streamer s;
    const int channel = 1;
    assert(!s.canPlay(channel));

    float x[6] = {6, 5, 4, 3, 2, 1};
    assertEQ(x[0], 6);

    s.setSample(channel, x, 6);
    s.setTranspose(channel, false, 1.f);
    assert(s.canPlay(channel));
    for (int i = 0; i < 6; ++i) {
        float_4 v = s.step();
        assertEQ(v[channel], 6 - i);
    }
    assert(!s.canPlay(channel));
}

static void testStreamXpose1() {
    Streamer s;
    const int channel = 3;
    assert(!s.canPlay(channel));

    float x[6] = {6, 5, 4, 3, 2, 1};
    assertEQ(x[0], 6);

    s.setSample(channel, x, 6);
    s.setTranspose(channel, true, 1.f);
    assert(s.canPlay(channel));
    s.step();
    assert(s.canPlay(channel));
}

// Now that we have cubic interpolation, this test no longer works.
// Need better ones.
static void testStreamXpose2() {
    Streamer s;
    const int channel = 3;
    assert(!s.canPlay(channel));

    float x[7] = {6, 5, 4, 3, 2, 1, 0};
    assertEQ(x[0], 6);

    s.setSample(channel, x, 7);
    s.setTranspose(channel, true, 2.f);
    assert(s.canPlay(channel));
    for (int i = 0; i < 3; ++i) {
        float_4 v = s.step();
        // start with 5, as interpoator forces us to start on second sample
        printf("i = %d v=%f\n", i, v[channel]);
        assertEQ(v[channel], 5 - (2 * i));
    }
    assert(!s.canPlay(channel));
}

static void testStreamRetrigger() {
    printf("testStreamRetrigger\n");
    Streamer s;
    const int channel = 0;

    float x[6] = {6, 5, 4, 3, 2, 1};

    s.setSample(channel, x, 6);
    s.setTranspose(channel, false, 1.f);
    assert(s.canPlay(channel));
    for (int i = 0; i < 6; ++i) {
        float_4 v = s.step();
    }
    assert(!s.canPlay(channel));

    s.setSample(channel, x, 6);
    assert(s.canPlay(channel));
    for (int i = 0; i < 6; ++i) {
        assert(s.canPlay(channel));
        float_4 v = s.step();
    }
    assert(!s.canPlay(channel));
}

static void testSampler() {
    Sampler4vx s;
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    assert(errc.empty());
    WaveLoaderPtr w = std::make_shared<WaveLoader>();

    s.setLoader(w);
    s.setNumVoices(1);
    s.setPatch(cinst);

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s.note_on(channel, midiPitch, midiVel, 0);

    float_4 x = s.step(0, 1.f / 44100.f);
    assert(x[0] == 0);
}

static void testSamplerRealSound() {
    Sampler4vx s;
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    WaveLoaderPtr w = std::make_shared<WaveLoader>();
    cinst->_setTestMode();

    const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\samples\C4vH.wav)foo";
    w->addNextSample(p);
    w->load();

    WaveLoader::WaveInfoPtr info = w->getInfo(1);
    assert(info->valid);

    s.setLoader(w);
    s.setNumVoices(1);
    s.setPatch(cinst);

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s.note_on(channel, midiPitch, midiVel, 0);
    float_4 x = s.step(0, 1.f / 44100.f);
    assert(x[0] == 0);

    x = s.step(0, 1.f / 44100.f);
    assert(x[0] != 0);
}

static void testCIKeysAndValues(const std::string& pitch, int expectedPitch) {

    SKeyValuePairPtr p = std::make_shared<SKeyValuePair>("hikey", pitch);
    SKeyValueList l = {p};

    SamplerErrorContext errc;
    auto output = SamplerSchema::compile(errc, l);
    assert(errc.empty());
    assertEQ(output->_size(), 1);
    SamplerSchema::ValuePtr vp = output->get(SamplerSchema::Opcode::HI_KEY);
    assert(vp);
    assertEQ(vp->numericInt, expectedPitch);    
}

static void testCIKeysAndValues() {
    testCIKeysAndValues("12", 12);
}

static void testCIKeysAndValuesNotes()  {
        // c0 = 12
    // c6 - 84
    testCIKeysAndValues("c6",  12 * (6 + 1));
}

static void testCIKeysAndValuesNotesSharp()  {
    int expectedPitch = 12 * (4 + 1) + 2;
     testCIKeysAndValues("d#4", expectedPitch);
}

 

static void testParseGlobalAndRegionCompiled() {
    printf("start test parse global\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global><region>", inst);

    assert(err.empty());
    SamplerErrorContext errc;
    CompiledInstrument::expandAllKV(errc, inst);
    assert(errc.empty());

    assert(inst->global.compiledValues);
    assertEQ(inst->global.compiledValues->_size(), 0);

    SGroupPtr group = inst->groups[0];
    assert(group);
    assert(group->compiledValues);
    assertEQ(group->compiledValues->_size(), 0);
}

static void testParseGlobalWithKVAndRegionCompiled() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global>hikey=57<region>", inst);

    assert(err.empty());
    SamplerErrorContext errc;
    CompiledInstrument::expandAllKV(errc, inst);
    assert(errc.empty());

    assert(inst->global.compiledValues);
    assertEQ(inst->global.compiledValues->_size(), 1);
    auto val = inst->global.compiledValues->get(SamplerSchema::Opcode::HI_KEY);
    assertEQ(val->numericInt, 57);

    SGroupPtr group = inst->groups[0];
    assert(group);
    assert(group->compiledValues);
    assertEQ(group->compiledValues->_size(), 0);
}

static void testParseGlobalWitRegionKVCompiled() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global><region><region>lokey=57<region>", inst);

    assert(err.empty());
    SamplerErrorContext errc;
    CompiledInstrument::expandAllKV(errc, inst);
    assert(errc.empty());

    assert(inst->global.compiledValues);
    assertEQ(inst->global.compiledValues->_size(), 0);

    SGroupPtr group = inst->groups[0];
    assert(group);
    assert(group->compiledValues);
    assertEQ(group->compiledValues->_size(), 0);

    assertEQ(group->regions.size(), 3);
    SRegionPtr r = group->regions[0];
    assertEQ(r->compiledValues->_size(), 0);
    r = group->regions[2];
    assertEQ(r->compiledValues->_size(), 0);
    r = group->regions[1];
    assertEQ(r->compiledValues->_size(), 1);

    auto val = r->compiledValues->get(SamplerSchema::Opcode::LO_KEY);
    assertEQ(val->numericInt, 57);
}

static void testParseControl() {
    SQINFO("------------- testParseControl");
    const char* data = R"foo(<control>
        default_path=Woodwinds\Bassoon\stac\
        <global>ampeg_attack=0.001 ampeg_release=3 ampeg_dynamic=1 volume=0
        <group> //Begin Group 1
        // lorand=0.0 hirand=0.5 
        group_label=gr_1
        <region>sample=PSBassoon_A1_v1_rr1.wav lokey=43 hikey=46 pitch_keycenter=45 lovel=1 hivel=62 volume=12
        )foo";

    SQINFO("took out random group. does that still work?");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);
    assert(err.empty());

    assertEQ(inst->groups.size(), 1);
    assertEQ(inst->groups[0]->regions.size(), 1);

    auto gp = inst->groups[0];
    auto rg =inst->groups[0]->regions[0];
#if 0
    SRegionPtr region = inst->groups[0]->regions[0];

    bool foundSample = false;
    std::string sample;
    for (int i = 0; i < region.values.size(); ++i) {
        SKeyValuePairPtr kv = region->values[i];
        if (kb->key == "sample")
    }
#endif

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    assert(errc.empty());
    std::vector<CompiledRegionPtr> regions;
   // cinst->getAllRegions(regions);
    cinst->_pool()._getAllRegions(regions);
    assertEQ(regions.size(), 1);
    CompiledRegionPtr creg = regions[0];
    SQINFO("IN test, creg = %p\n", creg.get());
    assertEQ(creg->lokey, 43);
    // TODO: does this test work?
}

static void testParseLabel() {
    const char* data = R"foo(<region>sw_label=abd def ghi)foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);
    assert(err.empty());
}

static void testCompileInst0() {
    printf("\n-- test comp inst 1\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global><region>lokey=50 hikey=50", inst);
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr i = CompiledInstrument::make(errc, inst);
    assert(errc.empty());

    VoicePlayInfo info;
    VoicePlayParameter params;
    info.sampleIndex = 0;
    assert(!info.valid);
    params.midiPitch = 50;
    params.midiVelocity = 60;
    i->play(info, params, nullptr, 0);
    assert(info.valid);  // this will fail until we implement a real compiler
    assertNE(info.sampleIndex, 0);
}

static void testCompileInst1() {
    printf("\n-- test comp inst 1\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global><region>lokey=50\nhikey=50\nsample=foo<region>lokey=60\nhikey=60\nsample=bar<region>lokey=70\nhikey=70\nsample=baz", inst);
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr i = CompiledInstrument::make(errc, inst);
    assert(errc.empty());

    VoicePlayInfo info;
    info.sampleIndex = 0;
    assert(!info.valid);
    VoicePlayParameter params;
    params.midiPitch = 60;
    params.midiVelocity = 60;
    i->play(info, params, nullptr, 0);
    assert(info.valid);  // this will fail until we implement a real compiler
    assertNE(info.sampleIndex, 0);
}

static void testCompileOverlap() {
    printf("\n-- test comp overlap\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(R"foo(<global>
        <region>lokey=0 hikey=127 sample=foo
        <region>lokey=60 hikey=60 pitch_keycenter=60 sample=bar)foo",
                          inst);
    assert(err.empty());

    SamplerErrorContext errc;
    CompiledInstrumentPtr ci = CompiledInstrument::make(errc, inst);
    assert(errc.empty());
    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 60;
    params.midiVelocity = 2;

    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertNE(info.sampleIndex, 0);
    assertEQ(info.needsTranspose, false);

    params.midiPitch = 61;
    params.midiVelocity = 100;
    ci->play(info, params, nullptr, 0);
    assert(!info.valid);
    params.midiPitch = 59;
    params.midiVelocity = 12;
    ci->play(info, params, nullptr, 0);
    assert(!info.valid);
}

static void testTranspose1() {
    printf("\nstarting on transpose 1\n");
    auto inst = std::make_shared<SInstrument>();
    auto err = SParse::go(R"foo(<region> sample=K18\D#1.pp.wav lovel=1 hivel=65 lokey=26 hikey=28 pitch_keycenter=27)foo", inst);
    assert(err.empty());
    SamplerErrorContext errc;
    auto cinst = CompiledInstrument::make(errc, inst);
    assert(errc.empty());
    VoicePlayInfo info;
    printf("about to fetch ifo for key = 26\n");

    // figure the expected transpose for pitch 26
    int semiOffset = -1;
    float pitchMul = float(std::pow(2, semiOffset / 12.0));

    VoicePlayParameter params;
    params.midiPitch = 26;
    params.midiVelocity = 64;
    cinst->play(info, params, nullptr, 0);
    assert(info.valid);
    assert(info.needsTranspose);
    assertEQ(info.transposeAmt, pitchMul);
}

static void testCubicInterp() {
    float data[] = {10, 9, 8, 7};

    // would need sample at -1 to interpolate sample 0
    assertEQ(CubicInterpolator<float>::canInterpolate(0, 4), false);
    assertEQ(CubicInterpolator<float>::canInterpolate(1, 4), true);
    assertEQ(CubicInterpolator<float>::canInterpolate(2, 4), false);
    assertEQ(CubicInterpolator<float>::canInterpolate(3, 4), false);

    assertEQ(CubicInterpolator<float>::canInterpolate(1.5f, 4), true);
    assertEQ(CubicInterpolator<float>::canInterpolate(1.99f, 4), true);

    float x = CubicInterpolator<float>::interpolate(data, 1);
    assertClose(x, 9, .00001);
    x = CubicInterpolator<float>::interpolate(data, 1.999f);
    assertClose(x, 8, .002);

    x = CubicInterpolator<float>::interpolate(data, 1.5f);
    assertClose(x, 8.5f, .0001);
}

static void testCompiledRegion() {
    SQINFO("---- starting testCompiledRegion");
    CompiledRegionPtr cr = st::makeRegion(R"foo(<region>sample=K18\C7.pp.wav lovel=1 hivel=22 lokey=95 hikey=97 pitch_keycenter=96 tune=10 offset=200)foo");
    assertEQ(cr->keycenter, 96);
    assertEQ(cr->lovel, 1);
    assertEQ(cr->hivel, 22);
    assertEQ(cr->lokey, 95);
    assertEQ(cr->hikey, 97);
    std::string expected = std::string("K18") + FilePath::nativeSeparator() + std::string("C7.pp.wav");
    assertEQ(cr->sampleFile, expected);
}

static void testCompiledRegionInherit() {
    SQINFO("---- starting testCompiledRegionInherit");
    CompiledRegionPtr cr = st::makeRegion(R"foo(<group>sample=K18\C7.pp.wav lovel=1 hivel=22 lokey=95 hikey=97 pitch_keycenter=96 tune=10 offset=200<region>)foo");
    assertEQ(cr->keycenter, 96);
    assertEQ(cr->lovel, 1);
    assertEQ(cr->hivel, 22);
    assertEQ(cr->lokey, 95);
    assertEQ(cr->hikey, 97);
    assertEQ(cr->sampleFile, "K18\\C7.pp.wav");
}

static void testCompiledRegionKey() {
    CompiledRegionPtr cr = st::makeRegion(R"foo(<region>key=32)foo");
    assertEQ(cr->lokey, 32);
    assertEQ(cr->hikey, 32);
    assertEQ(cr->keycenter, 32);
}

static void testCompiledRegionVel() {
    CompiledRegionPtr cr = st::makeRegion(R"foo(<region>hivel=12)foo");
    assertEQ(cr->lovel, 1);
    assertEQ(cr->hivel, 12);
}

static void testCompiledRegionVel2() {
    CompiledRegionPtr cr = st::makeRegion(R"foo(<region>lovel=71)foo");
    assertEQ(cr->lovel, 71);
    assertEQ(cr->hivel, 127);
}

static void testCompiledRegionVel3() {
    CompiledRegionPtr cr = st::makeRegion(R"foo(<region>hivel=59 lovel=29)foo");
    assertEQ(cr->lovel, 29);
    assertEQ(cr->hivel, 59);
}

static void testCompiledRegionsRand() {
    CompiledRegionPtr cr = st::makeRegion(R"foo(<region>hirand=.7 lorand=.29)foo");
    assertEQ(cr->hirand, .7f);
    assertEQ(cr->lorand, .29f);
}

static void testCompiledRegionSeqIndex1() {
    CompiledRegionPtr cr = st::makeRegion(R"foo(<region>seq_position=11)foo");
    assertEQ(cr->sequencePosition, 11);
}

static void testCompiledRegionSeqIndex2() {
    CompiledRegionPtr cr = st::makeRegion(R"foo(<group>seq_position=11<region>)foo");
    assertEQ(cr->sequencePosition, 11);
}
static void testCompiledGroupSub(const char* data, bool shouldIgnore) {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SGroupPtr group = inst->groups[0];
    SamplerErrorContext errc;
    CompiledInstrument::expandAllKV(errc, inst);
    assert(errc.empty());

    assert(inst->wasExpanded);

    CompiledGroupPtr cr = std::make_shared<CompiledGroup>(group);
    assertEQ(cr->shouldIgnore(), shouldIgnore);
}

static void testCompiledGroup0() {
    testCompiledGroupSub(R"foo(<group>)foo", false);
}

static void testCompiledGroup1() {
    testCompiledGroupSub(R"foo(<group>trigger=attack)foo", false);
}

static void testCompiledGroup2() {
    testCompiledGroupSub(R"foo(<group>trigger=release)foo", true);
}

static void testCompileMutliControls() {
    SQWARN("need to re-implement testCompileMutliControls");
}
#if 0   // need to re-do this
static void testCompileMutliControls() {
    SQINFO("--- start testCompileMutliControls");

    const char* test = R"foo(
        <control>
        default_path=a
        <region>
        sample=r1
        <control>
        default_path=b
        <region>
        sample=r2
    )foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(test, inst);
    assert(err.empty());
    auto ci = CompiledInstrument::make(inst);
    assert(ci);

  // don't have a tree anymore
    auto gps = (ci->_pool())._groups();
    assertEQ(gps.size(), 2);
    assertEQ(gps[0]->regions.size(), 1);

    CompiledRegionPtr r0 = gps[0]->regions[0];
    assert(r0);
    CompiledRegionPtr r1 = gps[1]->regions[0];
    assert(r1);


    std::string expected = std::string("a") + FilePath::nativeSeparator() + std::string("r1");
#if 0
    std::string xx = std::string("a");
    fprintf(stderr, "xx=%s\n", xx.c_str());
     xx += FilePath::nativeSeparator();
    fprintf(stderr, "xx=%s\n", xx.c_str());
 xx += std::string("r1");
    fprintf(stderr, "xx=%s\n", xx.c_str());
    #endif

   // SQINFO("at 635, exp=%s", expected.c_str());
    fprintf(stderr, "at 635, exp=%s", expected.c_str());

    fflush(stderr); fflush(stdout);
    assertEQ(r0->sampleFile, expected);
    
    expected = std::string("b") + FilePath::nativeSeparator() + std::string("r2");
    assertEQ(r1->sampleFile, expected);
}
#endif


static void testCompileTreeOne() {
    printf("\n----- testCompileTreeOne\n");
    const char* data = R"foo(<group><region>)foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    assert(errc.empty());
#if 0
    auto gps = ci->_pool()._groups();
    assertEQ(gps.size(), 1);
    assertEQ(gps[0]->regions.size(), 1);
#endif

    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 60;
    params.midiVelocity = 60;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
}


static void testCompileTreeTwo() {
    printf("\n----- testCompileTreeOne\n");
    const char* data = R"foo(<group>
        <region>key=4
        <region>key=5
        <group>
        <group>)foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    assert(errc.empty());

    SQWARN("need to re-write testCompileTreeTwo");
#if 0
    auto gps = ci->_pool()._groups();
    assertEQ(gps.size(), 3);
    assertEQ(gps[0]->regions.size(), 2);
    assertEQ(gps[1]->regions.empty(), true);
    assertEQ(gps[2]->regions.empty(), true);
#endif
}

static void testCompileKey() {
    const char* data = R"foo(<region>key=12)foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 12;
    params.midiVelocity = 60;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.needsTranspose, false);
}

static void testCompileMultiPitch() {
    const char* data = R"foo(
        <region>lokey=10 hikey=12 sample=a pitch_keycenter=11
        <region>lokey=13 hikey=15 sample=b pitch_keycenter=14
        <region>lokey=16 hikey=20 sample=c pitch_keycenter=18
    )foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    VoicePlayInfo info;

    VoicePlayParameter params;
    params.midiPitch = 9;
    params.midiVelocity = 60;
    ci->play(info, params, nullptr, 0);
    assert(!info.valid);

    params.midiPitch = 21;
    params.midiVelocity = 60;
    ci->play(info, params, nullptr, 0);
    assert(!info.valid);

    params.midiPitch = 0;
    params.midiVelocity = 60;
    ci->play(info, params, nullptr, 0);
    assert(!info.valid);

    params.midiPitch = 127;
    params.midiVelocity = 60;
    ci->play(info, params, nullptr, 0);
    assert(!info.valid);

    params.midiPitch = 11;
    params.midiVelocity = 60;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.needsTranspose, false);

    //  checking sample index is checking something that just happens to be true
    // If this breaks the test should be fixed.
    assertEQ(info.sampleIndex, 1);

    params.midiPitch = 12;
    params.midiVelocity = 60;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.needsTranspose, true);
    assertGT(info.transposeAmt, 1);

    params.midiPitch = 10;
    params.midiVelocity = 60;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.needsTranspose, true);
    assertLT(info.transposeAmt, 1);

    params.midiPitch = 13;
    params.midiVelocity = 60;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.needsTranspose, true);
    assertEQ(info.sampleIndex, 2);

    params.midiPitch = 20;
    params.midiVelocity = 60;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.needsTranspose, true);
    assertGT(info.transposeAmt, 1);
    assertEQ(info.sampleIndex, 3);
}

static void testCompileMultiVel() {
    printf("\n---- testCompileMultiVel\n");
    const char* data = R"foo(
        <region>key=10 sample=a hivel=20
        <region>key=10 sample=a lovel=21 hivel=90
        <region>key=10 sample=a lovel=91
    )foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    // ci->_dump(0);
    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 11;
    params.midiVelocity = 60;
    ci->play(info, params, nullptr, 0);
    assert(!info.valid);

    params.midiPitch = 10;
    params.midiVelocity = 1;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.sampleIndex, 1);
}

static void testCompileMulPitchAndVelSimple() {
    const char* data = R"foo(
        <region>key=10 sample=a hivel=20  sample=a
        <region>key=10 sample=a lovel=21 hivel=90  sample=b
        <region>key=10 sample=a lovel=91  sample=c

        <region>key=20 sample=a hivel=20  sample=d
        <region>key=20 sample=a lovel=21 hivel=90  sample=e
        <region>key=20 sample=a lovel=91  sample=f

        <region>key=30 sample=a hivel=20  sample=h
        <region>key=30 sample=a lovel=21 hivel=90  sample=i
        <region>key=30 sample=a lovel=91  sample=j
    )foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    VoicePlayInfo info;

    std::set<int> sampleIndicies;

    VoicePlayParameter params;
    params.midiPitch = 10;
    params.midiVelocity = 1;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    params.midiPitch = 10;
    params.midiVelocity = 21;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    params.midiPitch = 10;
    params.midiVelocity = 91;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    params.midiPitch = 20;
    params.midiVelocity = 1;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    params.midiPitch = 20;
    params.midiVelocity = 21;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    params.midiPitch = 20;
    params.midiVelocity = 91;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    params.midiPitch = 30;
    params.midiVelocity = 1;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    params.midiPitch = 30;
    params.midiVelocity = 21;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    params.midiPitch = 30;
    params.midiVelocity = 91;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    assertEQ(sampleIndicies.size(), 9);
    for (auto x : sampleIndicies) {
        assert(x >= 1);
        assert(x <= 9);
    }
}

static void testCompileMulPitchAndVelComplex1() {
    printf("\n----- testCompileMulPitchAndVelComplex1\n");
    const char* data = R"foo(
        <region>key=10 sample=a hivel=20  sample=a
        <region>key=20 sample=a lovel =10 hivel=24  sample=d
       
    )foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    VoicePlayInfo info;
    std::set<int> sampleIndicies;

    VoicePlayParameter params;
    params.midiPitch = 10;
    params.midiVelocity = 20;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    params.midiPitch = 20;
    params.midiVelocity = 22;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    assertEQ(sampleIndicies.size(), 2);
}

static void testCompileAmpegRelease() {
    printf("\n----- static void testCompileAmpegRelease() \n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    const char* data = R"foo(
        <group>ampeg_release=50
        <region>key=55
        <region>key=30 ampeg_release=10
        <region>key=40 ampeg_release=0
        <group>
        <region>key=20
        )foo";
    auto err = SParse::go(data, inst);
    assert(err.empty());

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    assert(errc.empty());

    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 55;
    params.midiVelocity = 127;

    // inherited
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.ampeg_release, 50);

    // set directly
    params.midiPitch = 30;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.ampeg_release, 10);

    // default
    params.midiPitch = 20;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.ampeg_release, .001f);
}

static void testCompileAmpVel() {
    printf("\n----- static void testCompileAmpVel() \n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    const char* data = R"foo(
        <group>amp_veltrack=50
        <region>key=55<region>key=30 amp_veltrack=100
        <region>key=40 amp_veltrack=0
        <region>key=10
        <group>
        <region>key=20
        )foo";
    auto err = SParse::go(data, inst);
    assert(err.empty());

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    assert(errc.empty());

    ci->_dump(0);

    VoicePlayInfo info;
    VoicePlayParameter params;

    // velrack = 100
    params.midiPitch = 30;
    params.midiVelocity = 127;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.gain, 1);

    params.midiVelocity = 64;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertClose(info.gain, .25f, .01f);

    // veltrack = 0
    params.midiPitch = 40;
    params.midiVelocity = 127;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.gain, 1);

    params.midiVelocity = 1;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertEQ(info.gain, 1);

    // default. veltrack should be 100
    params.midiPitch = 20;
    params.midiVelocity = 64;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertClose(info.gain, .25f, .01f);

    // veltrack 50, inherited
    params.midiPitch = 10;
    params.midiVelocity = 64;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertClose(info.gain, .56, .01f);  //number gotten from known-good.
                                        //but at least it's > .25 and < 1
}

static void testCompileMulPitchAndVelComplex2() {
    printf("\n----- testCompileMulPitchAndVelComplex2\n");
    const char* data = R"foo(
        <region>key=10 sample=a hivel=20  sample=a
        <region>key=10 sample=a lovel=21 hivel=90  sample=b
        <region>key=10 sample=a lovel=95  sample=c

        <region>key=20 sample=a hivel=24  sample=d
        <region>key=20 sample=a lovel=25 hivel=33  sample=e
        <region>key=20 sample=a lovel=91  sample=f

        <region>key=30 sample=a hivel=20  sample=h
        <region>key=30 sample=a lovel=34 hivel=90  sample=i
        <region>key=30 sample=a lovel=111  sample=j
    )foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    VoicePlayInfo info;
    std::set<int> sampleIndicies;
    // ci->_dump(0);

    VoicePlayParameter params;
    params.midiPitch = 10;
    params.midiVelocity = 20;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    params.midiPitch = 10;
    params.midiVelocity = 27;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);

    params.midiPitch = 10;
    params.midiVelocity = 91;
    ci->play(info, params, nullptr, 0);
    assert(!info.valid);

    params.midiPitch = 10;
    params.midiVelocity = 95;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    sampleIndicies.insert(info.sampleIndex);
}

static void testGroupInherit() {
    const char* data = R"foo(
        //snare =====================================
        <group> amp_veltrack=98 key=40 loop_mode=one_shot lovel=101 hivel=127  // snare1 /////
        <region> sample=a
    )foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    // ci->_dump(0);
    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 40;
    params.midiVelocity = 120;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);

    params.midiPitch = 41;
    params.midiVelocity = 120;
    ci->play(info, params, nullptr, 0);
    assert(!info.valid);

    // This doesn't work, becuase we don't have exclusive velocity zones.
    // ci->play(info, 40, 100);
    // assert(!info.valid);
}

static void testCompileSimpleDrum() {
    printf("\n\n----- testCompileSimpleDrum\n");
    const char* data = R"foo(
        //snare =====================================
        <group> amp_veltrack=98 key=40 loop_mode=one_shot lovel=101 hivel=127  // snare1 /////
        <region> sample=a lorand=0 hirand=0.3
        <region> sample=b lorand=0.3 hirand=0.6
        <region> sample=c lorand=0.6 hirand=1.0

        //snareStick =====================================
        <group> amp_veltrack=98 volume=-11 key=41 loop_mode=one_shot lovel=1 hivel=127 seq_length=3 
        <region> sample=d seq_position=1
        <region> sample=e seq_position=2
        <region> sample=f seq_position=3
    )foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
   // SQINFO("dumping drum patch");
   // ci->_dump(0);
  //  SQINFO("done with dump");
    VoicePlayInfo info;

    std::set<int> waves;
    for (int i = 0; i < 40; ++i) {
        VoicePlayParameter params;
        params.midiPitch = 40;
        params.midiVelocity = 110;
        ci->play(info, params, nullptr, 0);

        assert(info.valid);
        assert(info.sampleIndex > 0);
     //   printf("sample index = %d\n", info.sampleIndex);
        waves.insert(info.sampleIndex);
    }

    assertEQ(waves.size(), 3);

    waves.clear();
    assertEQ(waves.size(), 0);

    SQWARN("--- done with rand, now seq");

    VoicePlayParameter params;
    params.midiPitch = 41;
    params.midiVelocity = 64;
    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    waves.insert(info.sampleIndex);

    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    waves.insert(info.sampleIndex);

    ci->play(info, params, nullptr, 0);
    assert(info.valid);
    assertGE(info.sampleIndex, 1);
    waves.insert(info.sampleIndex);

    // three should play all of them
    assertEQ(waves.size(), 3);
}

// test sorting of regions.
// Also tests comiling velocity layers
static void testCompileSort() {
    const char* data = R"foo(<region>key=12 lovel=51<region>key=12 hivel=10<region>key=12 lovel=11 hivel=50)foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    std::vector<CompiledRegionPtr> regions;
    ci->_pool()._getAllRegions(regions);
    ci->_pool().sortByVelocity(regions);

    assertEQ(regions.size(), 3);
    assertEQ(regions[0]->lovel, 1);
    assertEQ(regions[0]->hivel, 10);

    assertEQ(regions[1]->lovel, 11);
    assertEQ(regions[1]->hivel, 50);

    assertEQ(regions[2]->lovel, 51);
    assertEQ(regions[2]->hivel, 127);
}

static void testComp(const std::string& path) {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::goFile(path, inst);
    if (!err.empty()) {
        SQWARN("parse ret %s", err.c_str());
    }
    assert(err.empty());
    //  go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    assert(ci);
    // ci->_dump(0);
    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 40;
    params.midiVelocity = 110;
    ci->play(info, params, nullptr, 0);
}

static void testCompileBassoon() {
    SQINFO("testCompileBassoon");
    testComp(R"foo(D:\samples\VSCO-2-CE-1.1.0\VSCO-2-CE-1.1.0\BassoonStac.sfz)foo");
}

static void testCompileGroupProbability() {
    const char* data = R"foo(
    
<group> 
lorand=0.0 hirand=0.5
<region>
sample=a.wav
lokey=43
hikey=46
pitch_keycenter=45
lovel=1
hivel=62
volume=12

<region>
sample=c.wav
key=100
lovel=1
hivel=62
volume=12

<group> 
lorand=0.5 hirand=1.5
<region>
sample=b.wav
lokey=43
hikey=46
pitch_keycenter=45
lovel=1
hivel=62
volume=12

<region>
sample=d.wav
key=100
lovel=1
hivel=62
volume=12
)foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    assert(errc.empty());
    assert(ci);
}

static void testCompileGroupProbability2() {
    const char* data = R"foo(
    
<group> 
lorand=0.0 hirand=0.5
<region>
sample=a.wav
lokey=43
hikey=46
pitch_keycenter=45
lovel=1
hivel=62
volume=12
<group> 
lorand=0.5 hirand=1.5
<region>
sample=b.wav
lokey=43
hikey=46
pitch_keycenter=45
lovel=1
hivel=62
volume=12
)foo";
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go(data, inst);

    SamplerErrorContext errc;
    auto ci = CompiledInstrument::make(errc, inst);
    assert(errc.empty());
    assert(ci);
}

static void testSampleRate() {
    WaveLoader w;
    // TODO: change back to k18 orig when test is done
    w.addNextSample("D:\\samples\\K18-Upright-Piano\\K18\\A0.f.wav");

    w.load();
    auto x = w.getInfo(1);
    assert(x->valid);
    assertEQ(x->sampleRate, 48000);
    assertEQ(x->numChannels, 1);  // loader already made mono

    CompiledRegionPtr cr1 = st::makeRegion(R"foo(<region>sample=a key=60 seq_position=200)foo");

    //  SimpleVoicePlayer(CompiledRegionPtr reg, int midiPitch, int sampleIndex) :

    SimpleVoicePlayer simplePlayer(cr1, 60, 1);

    VoicePlayInfo info;
    VoicePlayParameter params;
    params.midiPitch = 60;
    params.midiVelocity = 64;
    simplePlayer.play(info, params, &w, 44100);
    assert(info.valid);

    float expectedTranspose = 44100.f / 48000.f;
    assert(info.needsTranspose);
    assertClose(info.transposeAmt, expectedTranspose, .01);

    //  bool needsTranspose = false;
    //  float transposeAmt = 1;
}

void testx2() {
    assert(parseCount == 0);
    assert(compileCount == 0);
    testWaveLoader0();
    testWaveLoader1();
    testWaveLoaderNot44();
    testPlayInfo();

    // put here just for now
    testCompileMutliControls();

    testCubicInterp();

    testStream();
    testStreamEnd();
    testStreamValues();
    testStreamRetrigger();
    testStreamXpose1();

    // printf("fix testStreamXpose2\n");
    //testStreamXpose2();

    testCIKeysAndValues();
    testCIKeysAndValuesNotes();
    testCIKeysAndValuesNotesSharp();
    testParseGlobalAndRegionCompiled();
    testParseGlobalWithKVAndRegionCompiled();
    testParseGlobalWitRegionKVCompiled();
    testParseControl();
    testParseLabel();

    testCompiledRegion();
    testCompiledRegionInherit();
    testCompiledRegionKey();
    testCompiledRegionVel();
    testCompiledRegionVel2();
    testCompiledRegionVel3();
    testCompiledRegionsRand();
    testCompiledRegionSeqIndex1();
    testCompiledRegionSeqIndex2();
    testCompileAmpVel();
    testCompileAmpegRelease();

    testCompiledGroup0();
    testCompiledGroup1();
    testCompiledGroup2();

    testCompileMutliControls();

    // Let' put lots of very basic compilation tests here
    testCompileTreeOne();
    testCompileTreeTwo();
    testCompileKey();
    testCompileMultiPitch();
    testCompileMultiVel();
    testCompileMulPitchAndVelSimple();
    testCompileMulPitchAndVelComplex1();
    testCompileMulPitchAndVelComplex2();
    testCompileGroupProbability();
    testCompileBassoon();
    testGroupInherit();
    assertEQ(compileCount, 0);

    assertEQ(compileCount, 0);

    testCompileSort();

    testCompileInst0();
    testCompileInst1();
    testCompileOverlap();

    testLoadWavesPiano();

    testTranspose1();

    testSampler();
    //testSamplerRealSound();

    testSampleRate();

#ifdef _SFZ_RANDOM
    testCompileSimpleDrum();
#endif
    assertEQ(parseCount, 0);
    assertEQ(compileCount, 0);
}
