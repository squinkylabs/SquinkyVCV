
#include "SInstrument.h"
#include "Streamer.h"
#include "WaveLoader.h"
#include "asserts.h"

static void testWaveLoader0()
{
    WaveLoader w;
    w.load("fake file name");
    auto x = w.getInfo(0);
    printf("foo\n");
    assert(!x->valid);
}

static void testWaveLoader1()
{
    WaveLoader w;
    w.load("D:\\samples\\UprightPianoKW-small-SFZ-20190703\\samples\\A3vH.wav");
    auto x = w.getInfo(0);
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
// void setSample(float* data, int frames);
 //   void setTranspose(bool doTranspoe, float amount);

    float x[6] = {0};
    s.setSample(x, 6);
    assert(s.canPlay());
}

void testx2()
{
    testWaveLoader0();
    testWaveLoader1();
    testPlayInfo();

    printf("make testINfoPiano work\n");
    //testPlayInfoPiano();

    testStream();
}