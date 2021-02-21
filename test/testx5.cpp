

#include "CompiledInstrument.h"
#include "Sampler4vx.h"
#include "SamplerErrorContext.h"
#include "SInstrument.h"
#include "WaveLoader.h"

#include "asserts.h"


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
    cinst->_setTestMode(CompiledInstrument::Tests::MiddleC);

    const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\samples\C4vH.wav)foo";
    w->addNextSample(FilePath(p));
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

std::shared_ptr<Sampler4vx> makeTest() {
    std::shared_ptr<Sampler4vx> s = std::make_shared< Sampler4vx>();

  SInstrumentPtr inst = std::make_shared<SInstrument>();

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    WaveLoaderPtr w = std::make_shared<WaveLoader>();
    w->_setTestMode(WaveLoader::Tests::DCOneSec);

    cinst->_setTestMode(CompiledInstrument::Tests::MiddleC);      // I don't know what this test mode does now, but probably not enough?


    WaveLoader::WaveInfoPtr info = w->getInfo(1);
    assert(info->valid);

    s->setLoader(w);
    s->setNumVoices(1);
    s->setPatch(cinst);


    //const int channel = 0;
    //const int midiPitch = 60;
    //const int midiVel = 60;

    return s;
}

// This mostly tests that the test infrastructure works.
static void testSamplerTestOutput() {
    auto s = makeTest();
    
    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s->note_on(channel, midiPitch, midiVel, 0);

    const float sampleTime = 1.f / 44100.f;
    const float_4 gates = SimdBlocks::maskTrue();

    float_4 x = s->step(gates, sampleTime);
    x = s->step(gates, sampleTime);
    assert(x[0] > .1);
}




void testx5() {
      testSampler();
      testSamplerTestOutput();
    //testSamplerRealSound();
}