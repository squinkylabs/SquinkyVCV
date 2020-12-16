
#include "Sampler4vx.h"
#include "SInstrument.h"
#include "Streamer.h"
#include "WaveLoader.h"
#include "asserts.h"

static void testWaveLoader0()
{
    WaveLoader w;
    w.load("fake file name");
    auto x = w.getInfo(1);
    printf("foo\n");
    assert(!x->valid);

    x = w.getInfo(0);
    assert(!x);
}

static void testWaveLoader1()
{
    WaveLoader w;
    w.load("D:\\samples\\UprightPianoKW-small-SFZ-20190703\\samples\\A3vH.wav");
    auto x = w.getInfo(1);
    printf("foo\n");
    assert(x->valid);
}

static void testPlayInfo() {
    SVoicePlayInfo info;
    assertEQ(info.valid, false);
}

static void testPlayInfoPiano() {
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\UprightPianoKW-small-20190703.sfz)foo";
    auto err = SParse::goFile(p, inst);
    assert(err.empty());

    SVoicePlayInfo info;
    inst->getInfo(info, 60, 60);
    assert(info.valid); 
}

static void testStream()
{
    Streamer s;
    assert(!s.canPlay());
    s.step();

    float x[6] = {0};
    s.setSample(x, 6);
    assert(s.canPlay());
}

static void testStreamEnd()
{
    Streamer s;
    assert(!s.canPlay());

    float x[6] = {0};
    s.setSample(x, 6);
    assert(s.canPlay());
    for (int i=0; i< 6; ++i) {
        s.step();
    }
    assert(!s.canPlay());
}

static void testStreamValues()
{
    Streamer s;
    assert(!s.canPlay());

    float x[6] = {6,5,4,3,2,1};
    assertEQ(x[0], 6);

    s.setSample(x, 6);
    s.setTranspose(false, 0);
    assert(s.canPlay());
    for (int i=0; i< 6; ++i) {
        float v = s.step();
        assertEQ(v, 6-i);
    }
    assert(!s.canPlay());
}

static void testStreamRetrigger()
{
    Streamer s;

    float x[6] = { 6,5,4,3,2,1 };

    s.setSample(x, 6);
    s.setTranspose(false, 0);
    assert(s.canPlay());
    for (int i = 0; i < 6; ++i) {
        float v = s.step();
    }
    assert(!s.canPlay());

    s.setSample(x, 6);
    assert(s.canPlay());
    for (int i = 0; i < 6; ++i) {
        assert(s.canPlay());
        float v = s.step();
       
    }
    assert(!s.canPlay());
}


static void testSampler()
{
    Sampler4vx s;
    SInstrumentPtr inst = std::make_shared<SInstrument>();
    WaveLoaderPtr w = std::make_shared<WaveLoader>();

    s.setLoader(w);
    s.setNumVoices(1);
    s.setPatch(inst);

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
    WaveLoaderPtr w = std::make_shared<WaveLoader>();
    inst->_setTestMode();

    const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\samples\C4vH.wav)foo";
    w->load(p);

    WaveLoader::WaveInfoPtr info = w->getInfo(1);
    assert(info->valid);

    s.setLoader(w);
    s.setNumVoices(1);
    s.setPatch(inst);

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s.note_on(channel, midiPitch, midiVel);
    float_4 x = s.step();
    assert(x[0] == 0);

    x = s.step();
    assert(x[0] != 0);

}

void testx2()
{
    testWaveLoader0();
    testWaveLoader1();
    testPlayInfo();

    printf("make testINfoPiano work\n");
    //testPlayInfoPiano();

    testStream();
    testStreamEnd();
    testStreamValues();
    testStreamRetrigger();

    //testSampler();
    testSamplerRealSound();
}