
#include "Sampler4vx.h"
#include "SInstrument.h"
#include "Streamer.h"
#include "WaveLoader.h"
#include "asserts.h"
#include "CompiledInstrument.h"
//#include "SParse.h"

static void testWaveLoader0()
{
    WaveLoader w;
    w.addNextSample("fake file name");
    w.load();
    auto x = w.getInfo(1);
    printf("foo\n");
    assert(!x->valid);

    x = w.getInfo(0);
    assert(!x);
}

static void testWaveLoader1()
{
    WaveLoader w;
    w.addNextSample("D:\\samples\\UprightPianoKW-small-SFZ-20190703\\samples\\A3vH.wav");
    w.load();
    auto x = w.getInfo(1);
    printf("foo\n");
    assert(x->valid);
}

static void testPlayInfo() {
    ci::VoicePlayInfo info;
    assertEQ(info.valid, false);
}

static void testPlayInfoPiano() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\UprightPianoKW-small-20190703.sfz)foo";
    auto err = SParse::goFile(p, inst);
    assert(err.empty());

    ci::CompiledInstrumentPtr cinst = ci::CompiledInstrument::make(inst);
    ci::VoicePlayInfo info;
    cinst->getInfo(info, 60, 60);
    assert(info.valid); 
    int minSampleIndex = 200;
    int maxSampleIndex = -200;
    for (int pitch =21; pitch <= 108; ++ pitch) {
        info.valid = false;
        cinst->getInfo(info, pitch, 60);
        assert(info.valid);
        assert(info.canPlay());
        minSampleIndex = std::min(minSampleIndex, info.sampleIndex);
        maxSampleIndex = std::max(maxSampleIndex, info.sampleIndex);
    }

    cinst->getInfo(info, 20, 60);
    assert(!info.valid);
    cinst->getInfo(info, 109, 60);
    assert(!info.valid);

    assert(minSampleIndex == 1);
    assert(maxSampleIndex > 4);

}

static void testLoadWavesPiano()
{
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\UprightPianoKW-small-20190703.sfz)foo";
    auto err = SParse::goFile(p, inst);
    assert(err.empty());

    ci::CompiledInstrumentPtr cinst = ci::CompiledInstrument::make(inst);
    WaveLoaderPtr loader = std::make_shared<WaveLoader>();

    const char* pRoot = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\)foo";
    cinst->setWaves(loader, pRoot);
    loader->load();
    // assert(false);
}

static void testStream()
{
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

static void testStreamEnd()
{
    Streamer s;
    const int channel = 2;
    assert(!s.canPlay(channel));

    float x[6] = {0};
    s.setSample(channel, x, 6);
    assert(s.canPlay(channel));
    for (int i=0; i< 6; ++i) {
        s.step();
    }
    assert(!s.canPlay(channel));
}

static void testStreamValues()
{
    Streamer s;
    const int channel = 1;
    assert(!s.canPlay(channel));

    float x[6] = {6,5,4,3,2,1};
    assertEQ(x[0], 6);

    s.setSample(channel, x, 6);
    s.setTranspose(channel, false, 1.f);
    assert(s.canPlay(channel));
    for (int i=0; i< 6; ++i) {
        float_4 v = s.step();
        assertEQ(v[channel], 6-i);
    }
    assert(!s.canPlay(channel));
}

static void testStreamXpose()
{
    Streamer s;
    const int channel = 3;
    assert(!s.canPlay(channel));

    float x[6] = {6,5,4,3,2,1};
    assertEQ(x[0], 6);

    s.setSample(channel, x, 6);
    s.setTranspose(channel, true, 2.f);
    assert(s.canPlay(channel));
    for (int i=0; i< 3; ++i) {
        float_4 v = s.step();
        assertEQ(v[channel], 6-(2*i));
    }
    assert(!s.canPlay(channel));
}

static void testStreamRetrigger()
{
    Streamer s;
    const int channel = 0;

    float x[6] = { 6,5,4,3,2,1 };

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

static void testSampler()
{
    Sampler4vx s;
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    ci::CompiledInstrumentPtr cinst = ci::CompiledInstrument::make(inst);
    WaveLoaderPtr w = std::make_shared<WaveLoader>();

    s.setLoader(w);
    s.setNumVoices(1);
    s.setPatch(cinst);

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s.note_on(channel, midiPitch, midiVel);
    float_4 x = s.step();
    assert(x[0] == 0);
}

static void testSamplerRealSound()
{
    Sampler4vx s;
    SInstrumentPtr inst = std::make_shared<SInstrument>();

    ci::CompiledInstrumentPtr cinst = ci::CompiledInstrument::make(inst);
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
    s.note_on(channel, midiPitch, midiVel);
    float_4 x = s.step();
    assert(x[0] == 0);

    x = s.step();
    assert(x[0] != 0);
}

static void testCIKeysAndValues()
{
    SKeyValuePair p2 = { "hikey", "12" };

    SKeyValuePairPtr p = std::make_shared<SKeyValuePair>("hikey", "12");
    SKeyValueList l = { p };

 
    auto output = ci::compile(l);
    assertEQ(output->_size(), 1);
    ci::ValuePtr vp = output->get(ci::Opcode::HI_KEY);
    assert(vp);
    assertEQ(vp->numeric, 12);
}

static void testParseGlobalAndRegionCompiled()
{
    printf("start test parse global\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global><region>", inst);

    assert(err.empty());
    ci::expandAllKV(inst);
    assert(inst->global.compiledValues);
    assertEQ(inst->global.compiledValues->_size(), 0);

    SGroupPtr group = inst->groups[0];
    assert(group);
    assert(group->compiledValues);
    assertEQ(group->compiledValues->_size(), 0);
}

static void testParseGlobalWithKVAndRegionCompiled()
{
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global>hikey=57<region>", inst);

    assert(err.empty());
    ci::expandAllKV(inst);
    assert(inst->global.compiledValues);
    assertEQ(inst->global.compiledValues->_size(), 1);
    auto val  = inst->global.compiledValues->get(ci::Opcode::HI_KEY);
    assertEQ(val->numeric, 57);

    SGroupPtr group = inst->groups[0];
    assert(group);
    assert(group->compiledValues);
    assertEQ(group->compiledValues->_size(), 0);
}

static void testParseGlobalWitRegionKVCompiled()
{
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global><region><region>lokey=57<region>", inst);

    assert(err.empty());
    ci::expandAllKV(inst);
    assert(inst->global.compiledValues);
    assertEQ(inst->global.compiledValues->_size(), 0);

    SGroupPtr group = inst->groups[0];
    assert(group);
    assert(group->compiledValues);
    assertEQ(group->compiledValues->_size(), 0);

    assertEQ(group->regions.size(), 3)
    SRegionPtr r = group->regions[0];
    assertEQ(r->compiledValues->_size(), 0);
    r = group->regions[2];
    assertEQ(r->compiledValues->_size(), 0);
    r = group->regions[1];
    assertEQ(r->compiledValues->_size(), 1);

    auto val = r->compiledValues->get(ci::Opcode::LO_KEY);
    assertEQ(val->numeric, 57);
}

static void testCompileInst1()
{
    printf("\n-- test comp inst 1\n");
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto err = SParse::go("<global><region><region>lokey=60\nhikey=60\nsample=foo<region>", inst);
    assert(err.empty());

    ci::CompiledInstrumentPtr i = ci::CompiledInstrument::make(inst);

    ci::VoicePlayInfo info;
    info.sampleIndex = 0;
    assert(!info.valid);
    i->getInfo(info, 60, 60);
    assert(info.valid);  // this will fail until we implement a real compiler
    assertNE(info.sampleIndex, 0);
}

static void testTranspose1()
{
    auto inst = std::make_shared<SInstrument>();
    auto err = SParse::go(R"foo(<region> sample=K18\D#1.pp.wav lovel=1 hivel=22 lokey=26 hikey=28 pitch_keycenter=27)foo", inst);
    assert(err.empty());
    auto cinst = ci::CompiledInstrument::make(inst);
    ci::VoicePlayInfo info;
    printf("about to fetch ifo for key = 26\n");
    cinst->getInfo(info, 26, 64);
    assert(info.needsTranspose);
    assertEQ(info.transposeAmt, 26.f/27.f);

}


void testx2()
{
    testWaveLoader0();
    testWaveLoader1();
    testPlayInfo();

    testStream();
    testStreamEnd();
    testStreamValues();
    testStreamRetrigger();
    testStreamXpose();

    //testSampler();
    testSamplerRealSound();

    testCIKeysAndValues();
    testParseGlobalAndRegionCompiled();
    testParseGlobalWithKVAndRegionCompiled();
    testParseGlobalWitRegionKVCompiled();

    testCompileInst1();
    testPlayInfoPiano();
    testLoadWavesPiano();

    printf("fix test transpose 1\n");
    //testTranspose1();

}