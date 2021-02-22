

#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "Sampler4vx.h"
#include "SamplerErrorContext.h"
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
    std::shared_ptr<Sampler4vx> s = std::make_shared<Sampler4vx>();

    SInstrumentPtr inst = std::make_shared<SInstrument>();

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    WaveLoaderPtr w = std::make_shared<WaveLoader>();
    w->_setTestMode(WaveLoader::Tests::DCOneSec);

    cinst->_setTestMode(CompiledInstrument::Tests::MiddleC);  // I don't know what this test mode does now, but probably not enough?

    WaveLoader::WaveInfoPtr info = w->getInfo(1);
    assert(info->valid);

    s->setLoader(w);
    s->setNumVoices(1);
    s->setPatch(cinst);

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

using ProcFunc = std::function<float()>;

static unsigned measureAttack( ProcFunc f, float threshold) {
    unsigned int ret = 0;
    float x = f();
    assert (x < .5);
    ret++;

    while (x < threshold) {
        ++ret;
        x = f();
    }
    return ret;
}

static unsigned measureRelease( ProcFunc f, float threshold) {
    unsigned int ret = 0;
    float x = f();
  //  assert (x > .5);
    ret++;

    while (x > threshold) {
        ++ret;
        x = f();
    }
    return ret;
}

static void testSamplerAttack() {
    auto s = makeTest();

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s->note_on(channel, midiPitch, midiVel, 0);

    float_4 gates = SimdBlocks::maskTrue();
    ProcFunc lambda = [s, &gates] {
        const float sampleTime = 1.f / 44100.f;
     
        const float_4 x = s->step(gates, sampleTime);
        return x[0];
    };

    const auto attackSamples = measureAttack(lambda, .95f * Sampler4vx::_outputGain()[0]);

    // These are arbitrary "known good" values,
    // but the point is to be sure the default attack is "fast"
    assert(attackSamples > 100);
    assert(attackSamples < 200);
}

static void testSamplerRelease() {
    auto s = makeTest();

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s->note_on(channel, midiPitch, midiVel, 0);

    float_4 gates = SimdBlocks::maskTrue();
    ProcFunc lambda = [s, &gates] {
        const float sampleTime = 1.f / 44100.f;

        const float_4 x = s->step(gates, sampleTime);
        return x[0];
    };

    auto attackSamples = measureAttack(lambda, .95f * Sampler4vx::_outputGain()[0]);
    gates = SimdBlocks::maskFalse();
    const auto releaseSamples = measureRelease(lambda, .05f * Sampler4vx::_outputGain()[0]);

    // These are arbitrary "known good" values,
    // but the point is to be sure the default attack is "fast"
    // I get 20,800 when I cheat. about right
    assertLT(releaseSamples, 22000);
    assertGT(releaseSamples, 18000);
}

// validate that the release envelope kicks in a the end of the sampl/
static void testSamplerEnd() {
    auto s = makeTest();

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s->note_on(channel, midiPitch, midiVel, 0);

    float_4 gates = SimdBlocks::maskTrue();
    ProcFunc lambda = [s, &gates] {
        const float sampleTime = 1.f / 44100.f;

        const float_4 x = s->step(gates, sampleTime);
        return x[0];
    };

    auto attackSamples = measureAttack(lambda, .99f * Sampler4vx::_outputGain()[0]);
    // don't lower the gate, just let it end
    // masure when it starts to go down
    const auto releaseSamples = measureRelease(lambda, .95f * Sampler4vx::_outputGain()[0]);
    // and finish
    const auto releaseSamples2 = measureRelease(lambda, .05f * Sampler4vx::_outputGain()[0]);


    assertGT(releaseSamples2, 18000);
    assertLT(releaseSamples2, 22000);
}

void testx5() {
    testSampler();
    testSamplerTestOutput();
    testSamplerAttack();
    testSamplerRelease();
    testSamplerEnd();
    //testSamplerRealSound();
}