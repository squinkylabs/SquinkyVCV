

#include <memory>

#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "Samp.h"
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

    float_4 x = s.step(0, 1.f / 44100.f, 0, false);
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
    w->load2();

    WaveLoader::WaveInfoPtr info = w->getInfo(1);
    assert(info->valid);

    s.setLoader(w);
    s.setNumVoices(1);
    s.setPatch(cinst);

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s.note_on(channel, midiPitch, midiVel, 0);
    float_4 x = s.step(0, 1.f / 44100.f, 0, false);
    assert(x[0] == 0);

    x = s.step(0, 1.f / 44100.f, 0, false);
    assert(x[0] != 0);
}

// CompiledInstrument::Tests::MiddleC
// WaveLoader::Tests::DCOneSec
std::shared_ptr<Sampler4vx> makeTest(CompiledInstrument::Tests citest, WaveLoader::Tests wltest) {
    std::shared_ptr<Sampler4vx> s = std::make_shared<Sampler4vx>();

    SInstrumentPtr inst = std::make_shared<SInstrument>();

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    WaveLoaderPtr w = std::make_shared<WaveLoader>();
    w->_setTestMode(wltest);

    cinst->_setTestMode(citest);  // I don't know what this test mode does now, but probably not enough?

    WaveLoader::WaveInfoPtr info = w->getInfo(1);
    assert(info->valid);

    s->setLoader(w);
    s->setNumVoices(1);
    s->setPatch(cinst);

    return s;
}

// This mostly tests that the test infrastructure works.
static void testSamplerTestOutput() {
    SQINFO("---- testSamplerTestOutput");
    auto s = makeTest(CompiledInstrument::Tests::MiddleC, WaveLoader::Tests::DCOneSec);

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s->note_on(channel, midiPitch, midiVel, 0);

    const float sampleTime = 1.f / 44100.f;
    const float_4 gates = SimdBlocks::maskTrue();

    float_4 x = s->step(gates, sampleTime, 0, false);
    x = s->step(gates, sampleTime, 0, false);
    assertGE(x[0], .01);
}

using ProcFunc = std::function<float()>;

static unsigned measureAttack(ProcFunc f, float threshold) {
    unsigned int ret = 0;
    float x = f();
    // assert (x < .5);
    assert(x < Sampler4vx::_outputGain()[0] / 2);
    ret++;

    const int maxIterations = 44100 * 20;  // 20 second time out
    while (x < threshold) {
        ++ret;
        x = f();
        assertLT(ret, maxIterations);
    }
    return ret;
}

static unsigned measureRelease(ProcFunc f, float threshold) {
    unsigned int ret = 0;
    float x = f();
    // assert (x > .5);
    assert(x > Sampler4vx::_outputGain()[0] / 2);
    ret++;

    while (x > threshold) {
        ++ret;
        x = f();
    }
    return ret;
}

static void testSamplerAttack() {
    SQINFO("----- testSamplerAttack");
    auto s = makeTest(CompiledInstrument::Tests::MiddleC, WaveLoader::Tests::DCOneSec);

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s->note_on(channel, midiPitch, midiVel, 0);

    float_4 gates = SimdBlocks::maskTrue();
    ProcFunc lambda = [s, &gates] {
        const float sampleTime = 1.f / 44100.f;

        const float_4 x = s->step(gates, sampleTime, 0, false);
        return x[0];
    };

    const auto attackSamples = measureAttack(lambda, .95f * Sampler4vx::_outputGain()[0]);

    // These are arbitrary "known good" values,
    // but the point is to be sure the default attack is "fast"
    assertGT(attackSamples, 100);
    assertLT(attackSamples, 200);
}

static void prime(std::shared_ptr<Sampler4vx> s) {
    const float sampleTime = 1.f / 44100.f;
    const float_4 zero = SimdBlocks::maskFalse();
    s->step(zero, sampleTime, 0, false);
    s->step(zero, sampleTime, 0, false);
}

static void testSamplerRelease() {
    SQINFO("----- testSamplerRelease");
    auto s = makeTest(CompiledInstrument::Tests::MiddleC, WaveLoader::Tests::DCOneSec);

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s->note_on(channel, midiPitch, midiVel, 0);
    prime(s);  // probably no needed, but  a few quiet ones to start

    float_4 gates = SimdBlocks::maskTrue();
    ProcFunc lambda = [s, &gates] {
        const float sampleTime = 1.f / 44100.f;

        const float_4 x = s->step(gates, sampleTime, 0, false);
        return x[0];
    };

    auto attackSamples = measureAttack(lambda, .95f * Sampler4vx::_outputGain()[0]);
    gates = SimdBlocks::maskFalse();
    const float minus85Db = (float)AudioMath::gainFromDb(-85);
    const float releaseMeasureThreshold = minus85Db * Sampler4vx::_outputGain()[0];
    const auto releaseSamples = measureRelease(lambda, releaseMeasureThreshold);

    // I think .6 should give me about 26k samples,
    const float f = .6 * 44100.f;
    assertClosePct(releaseSamples, f, 10);
}

// this one should have a 1.1 second release
static void testSamplerRelease2() {
    auto s = makeTest(CompiledInstrument::Tests::MiddleC11, WaveLoader::Tests::DCTenSec);

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s->note_on(channel, midiPitch, midiVel, 0);

    float_4 gates = SimdBlocks::maskTrue();
    ProcFunc lambda = [s, &gates] {
        const float sampleTime = 1.f / 44100.f;

        const float_4 x = s->step(gates, sampleTime, 0, false);
        return x[0];
    };

    auto attackSamples = measureAttack(lambda, .95f * Sampler4vx::_outputGain()[0]);
    gates = SimdBlocks::maskFalse();

    const float minus85Db = (float)AudioMath::gainFromDb(-85);
    const float releaseMeasureThreshold = minus85Db * Sampler4vx::_outputGain()[0];
    const auto releaseSamples = measureRelease(lambda, releaseMeasureThreshold);

    const float f = 1.1f * 44100.f;
    assertClosePct(releaseSamples, f, 10);
}

// validate that the release envelope kicks in a the end of the sample
// no longer valid: that feature removed
static void testSamplerEnd() {
    assert(false);
    auto s = makeTest(CompiledInstrument::Tests::MiddleC, WaveLoader::Tests::DCOneSec);
    prime(s);

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s->note_on(channel, midiPitch, midiVel, 0);

    float_4 gates = SimdBlocks::maskTrue();
    ProcFunc lambda = [s, &gates] {
        const float sampleTime = 1.f / 44100.f;

        const float_4 x = s->step(gates, sampleTime, 0, false);
        return x[0];
    };

    const float minus85Db = (float)AudioMath::gainFromDb(-85);
    const float releaseMeasureThreshold = minus85Db * Sampler4vx::_outputGain()[0];

    auto attackSamples = measureAttack(lambda, .99f * Sampler4vx::_outputGain()[0]);
    // don't lower the gate, just let it end
    // masure when it starts to go down
    const auto releaseSamples = measureRelease(lambda, .95f * Sampler4vx::_outputGain()[0]);
    // and finish
    const auto releaseSamples2 = measureRelease(lambda, releaseMeasureThreshold);

    const float f = .6 * 44100.f;
    assertClosePct(releaseSamples2, f, 10);
}

// no longer valide since no env at end
// perhaps could be re-written
static void testSampleRetrigger() {
    SQINFO("------ testSampleRetrigger");
    auto s = makeTest(CompiledInstrument::Tests::MiddleC, WaveLoader::Tests::DCOneSec);
    prime(s);

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s->note_on(channel, midiPitch, midiVel, 0);

    float_4 gates = SimdBlocks::maskTrue();
    ProcFunc lambda = [s, &gates] {
        const float sampleTime = 1.f / 44100.f;

        const float_4 x = s->step(gates, sampleTime, 0, false);
        return x[0];
    };

    //--------------------- first, measure tirgger though to play-out
    const float minus85Db = (float)AudioMath::gainFromDb(-85);
    const float releaseMeasureThreshold = minus85Db * Sampler4vx::_outputGain()[0];

    auto attackSamples = measureAttack(lambda, .99f * Sampler4vx::_outputGain()[0]);

    // don't lower the gate, just let it end
    // masure when it starts to go down
    const auto releaseSamples = measureRelease(lambda, .95f * Sampler4vx::_outputGain()[0]);
    // and finish
    const auto releaseSamples2 = measureRelease(lambda, releaseMeasureThreshold);

    const float f = .6 * 44100.f;
    assertClosePct(releaseSamples2, f, 10);

    //------------------- second - re-trigger it a couple of times
    for (int i = 0; i < 4; ++i) {
        prime(s);                                    // send it a  few gate low to reset the ADSR
        s->note_on(channel, midiPitch, midiVel, 0);  // and re-trigger
        auto attackSamplesNext = measureAttack(lambda, .99f * Sampler4vx::_outputGain()[0]);
        assertEQ(attackSamplesNext, attackSamples);

        const auto releaseSamplesNext = measureRelease(lambda, .95f * Sampler4vx::_outputGain()[0]);
        // and finish
        const auto releaseSamplesNext2 = measureRelease(lambda, releaseMeasureThreshold);
        assertEQ(releaseSamplesNext2, releaseSamples2);
    }
}

static void testSampQantizer() {
    using Comp = Samp<TestComposite>;

    const float semiV = 1.f / 12.f;
    const float quarterV = semiV / 2.f;
    const float tinny = quarterV / 16.f;
    // 0v quantized to middle C
    assertEQ(Comp::quantize(0), 60);
    assertEQ(Comp::quantize(0 + quarterV - tinny), 60);
    assertEQ(Comp::quantize(0 + quarterV + tinny), 61);
    assertEQ(Comp::quantize(0 - quarterV + tinny), 60);
    assertEQ(Comp::quantize(0 - quarterV - tinny), 59);

    assertEQ(Comp::quantize(0 + 1 * semiV), 61);
    assertEQ(Comp::quantize(0 + 2 * semiV), 62);
    assertEQ(Comp::quantize(0 + 3 * semiV), 63);
    assertEQ(Comp::quantize(0 + 4 * semiV), 64);
    assertEQ(Comp::quantize(0 + 5 * semiV), 65);

    assertEQ(Comp::quantize(1 + 1 * semiV), 61 + 12);
    assertEQ(Comp::quantize(1 + 2 * semiV), 62 + 12);
    assertEQ(Comp::quantize(1 + 3 * semiV), 63 + 12);
    assertEQ(Comp::quantize(1 + 4 * semiV), 64 + 12);
    assertEQ(Comp::quantize(1 + 5 * semiV), 65 + 12);

    assertEQ(Comp::quantize(-1 + 1 * semiV), 61 - 12);
    assertEQ(Comp::quantize(-1 + 2 * semiV), 62 - 12);
    assertEQ(Comp::quantize(-1 + 3 * semiV), 63 - 12);
    assertEQ(Comp::quantize(-1 + 4 * semiV), 64 - 12);
    assertEQ(Comp::quantize(-1 + 5 * semiV), 65 - 12);
}

// This test is just to force compile errors in Samp.h
// Later, when there are real tests for Samp, this could go away
static void testSampBUilds() {
    using Comp = Samp<TestComposite>;
    Comp::ProcessArgs arg;
    std::shared_ptr<Comp> pcomp = std::make_shared<Comp>();
    pcomp->init();
    pcomp->process(arg);
}

void testx5() {
    testSampler();
    testSamplerTestOutput();

    printf("put back all of these!\n");
    // testSamplerAttack();
    testSamplerRelease();
    // testSamplerEnd();
    //testSamplerRealSound();

    printf("put back test release 2!!!!\n");
    testSamplerRelease2();

    testSampQantizer();
    testSampBUilds();

    // testSampleRetrigger();      // now write a test for retriggering played out voice
}