

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
    w->loadNextFile();

    WaveLoader::WaveInfoPtr info = w->getInfo(1);
    assert(info->isValid());

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
std::shared_ptr<Sampler4vx> makeTestSampler4vx(CompiledInstrument::Tests citest, WaveLoader::Tests wltest) {
    std::shared_ptr<Sampler4vx> s = std::make_shared<Sampler4vx>();

    SInstrumentPtr inst = std::make_shared<SInstrument>();

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst = CompiledInstrument::make(errc, inst);
    WaveLoaderPtr w = std::make_shared<WaveLoader>();
    w->_setTestMode(wltest);

    cinst->_setTestMode(citest);  // I don't know what this test mode does now, but probably not enough?

    WaveLoader::WaveInfoPtr info = w->getInfo(1);
    assert(info->isValid());

    s->setLoader(w);
    s->setNumVoices(1);
    s->setPatch(cinst);

    return s;
}

// This mostly tests that the test infrastructure works.
static void testSampler4vxTestOutput() {
    auto s = makeTestSampler4vx(CompiledInstrument::Tests::MiddleC, WaveLoader::Tests::DCOneSec);

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

static unsigned sampler4vxMeasureAttack(ProcFunc f, float threshold) {
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

static unsigned sampler4vxMeasureRelease(ProcFunc f, float threshold) {
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

static void testSampler4vxAttack() {
    auto s = makeTestSampler4vx(CompiledInstrument::Tests::MiddleC, WaveLoader::Tests::DCOneSec);

    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;

    const float sampleTime = 1.f / 44100.f;
    float_4 lowGates = float_4::zero();
    s->step(lowGates, sampleTime, 0, false);
    s->note_on(channel, midiPitch, midiVel, 0);

    float_4 gates = SimdBlocks::maskTrue();
    ProcFunc lambda = [s, &gates, sampleTime] {
        const float_4 x = s->step(gates, sampleTime, 0, false);
        return x[0];
    };

    const auto attackSamples = sampler4vxMeasureAttack(lambda, .95f * Sampler4vx::_outputGain()[0]);

    // default for attack is 1 ms (no it isn't)
    const int expectedAttack = 44100 / 1000;
    assertClosePct(attackSamples, expectedAttack, 10);
}

static void prime(std::shared_ptr<Sampler4vx> s) {
    const float sampleTime = 1.f / 44100.f;
    const float_4 zero = SimdBlocks::maskFalse();
    s->step(zero, sampleTime, 0, false);
    s->step(zero, sampleTime, 0, false);
}

static void testSampler4vxRelease() {
    auto s = makeTestSampler4vx(CompiledInstrument::Tests::MiddleC, WaveLoader::Tests::DCOneSec);

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

    auto attackSamples = sampler4vxMeasureAttack(lambda, .95f * Sampler4vx::_outputGain()[0]);
    gates = SimdBlocks::maskFalse();
    const float minus85Db = (float)AudioMath::gainFromDb(-85);
    const float releaseMeasureThreshold = minus85Db * Sampler4vx::_outputGain()[0];
    const auto releaseSamples = sampler4vxMeasureRelease(lambda, releaseMeasureThreshold);

    // .6 is what Tests::MiddleC uses for release
    const float f = .6 * 44100.f;
    assertClosePct(releaseSamples, f, 10);
}

// this one should have a 1.1 second release
static void testSamplerRelease2() {
    auto s = makeTestSampler4vx(CompiledInstrument::Tests::MiddleC11, WaveLoader::Tests::DCTenSec);

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

    auto attackSamples = sampler4vxMeasureAttack(lambda, .95f * Sampler4vx::_outputGain()[0]);
    gates = SimdBlocks::maskFalse();

    const float minus85Db = (float)AudioMath::gainFromDb(-85);
    const float releaseMeasureThreshold = minus85Db * Sampler4vx::_outputGain()[0];
    const auto releaseSamples = sampler4vxMeasureRelease(lambda, releaseMeasureThreshold);

    const float f = 1.1f * 44100.f;
    assertClosePct(releaseSamples, f, 10);
}

// validate that the release envelope kicks in a the end of the sample
// no longer valid: that feature removed
#if 0
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
#endif

static void testSampler4vxRetrigger() {
    auto s = makeTestSampler4vx(CompiledInstrument::Tests::MiddleC, WaveLoader::Tests::DCOneSec);
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

    auto attackSamples = sampler4vxMeasureAttack(lambda, .99f * Sampler4vx::_outputGain()[0]);

    // don't lower the gate, just let it end
    // masure when it starts to go down
    const auto releaseSamples = sampler4vxMeasureRelease(lambda, .95f * Sampler4vx::_outputGain()[0]);
    // and finish
    const auto releaseSamples2 = sampler4vxMeasureRelease(lambda, releaseMeasureThreshold);

    const float f = .6 * 44100.f;
    assertClosePct(releaseSamples2, f, 10);

    //------------------- second - re-trigger it a couple of times
    for (int i = 0; i < 4; ++i) {
        prime(s);                                    // send it a  few gate low to reset the ADSR
        s->note_on(channel, midiPitch, midiVel, 0);  // and re-trigger
        auto attackSamplesNext = sampler4vxMeasureAttack(lambda, .99f * Sampler4vx::_outputGain()[0]);
        assertEQ(attackSamplesNext, attackSamples);

        const auto releaseSamplesNext = sampler4vxMeasureRelease(lambda, .95f * Sampler4vx::_outputGain()[0]);
        // and finish
        const auto releaseSamplesNext2 = sampler4vxMeasureRelease(lambda, releaseMeasureThreshold);
        assertEQ(releaseSamplesNext2, releaseSamples2);
    }
}

// tests the semitone quantizer
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
static void testSampBuilds() {
    using Comp = Samp<TestComposite>;
    Comp::ProcessArgs arg;
    std::shared_ptr<Comp> pcomp = std::make_shared<Comp>();
    pcomp->suppressErrors();
    pcomp->init();
    pcomp->process(arg);
}

static void testSampPitch0(int channel) {
    auto s = makeTestSampler4vx(CompiledInstrument::Tests::MiddleC, WaveLoader::Tests::DCOneSec);

    // const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    s->note_on(channel, midiPitch, midiVel, 0);
    const bool isTrans = s->_isTransposed(channel);
    const float transAmt = s->_transAmt(channel);

    assert(!isTrans);
    assertEQ(transAmt, 1.f);
}

static void testSampPitch1(int channel) {
    auto s = makeTestSampler4vx(CompiledInstrument::Tests::MiddleC, WaveLoader::Tests::DCOneSec);
    const int midiPitch = 60;
    const int midiVel = 60;

    s->note_on(channel, midiPitch, midiVel, 0);

    float_4 mod = float_4::zero();
    mod[channel] = 1;  // let's go up an octabe (1V/8)
    s->setExpFM(mod);
    for (int i = 0; i < 4; ++i) {
        const bool isTrans = s->_isTransposed(i);
        const float transAmt = s->_transAmt(i);

        assertEQ(isTrans, (i == channel));
        const float expectedTrans = (i == channel) ? 2.f : 1.f;
        assertClose(transAmt, expectedTrans, .0001f);
    }
}

static void testSampPitch() {
    for (int i = 0; i < 4; ++i) {
        testSampPitch0(i);
        testSampPitch1(i);
    }
}

static void testSampOffset() {
    std::shared_ptr<Sampler4vx> s = std::make_shared<Sampler4vx>();

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst;
    WaveLoaderPtr w = std::make_shared<WaveLoader>();
    w->_setTestMode(WaveLoader::Tests::RampOneSec);

    const char* test = R"foo(
        <region>
        sample=r1
        offset=4321
    )foo";

    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto perr = SParse::go(test, inst);
    assert(perr.empty());

    SamplerErrorContext cerr;
    auto ci = CompiledInstrument::make(cerr, inst);
    assert(ci);

    VoicePlayParameter params;
    params.midiPitch = 60;
    params.midiVelocity = 60;

    Sampler4vx samp;
    samp.setLoader(w);
    samp.setPatch(ci);

    VoicePlayInfo info;
    ci->play(info, params, nullptr, 0);
    assertEQ(info.loopData.offset, 4321);
    SQINFO("foo");

    samp.note_on(0, 60, 60, 44100);
    const Streamer& player = samp._player();
    assertEQ(player.channels[0].loopData.offset, 4321);
    assertEQ(player.channels[0].loopActive, false);
}

static void testOneShot(bool oneShot) {
    SQINFO("--- testOneShot(%d)", oneShot);
    //std::shared_ptr<Sampler4vx> s = std::make_shared<Sampler4vx>();

    SamplerErrorContext errc;
    CompiledInstrumentPtr cinst;
    WaveLoaderPtr w = std::make_shared<WaveLoader>();
    w->_setTestMode(WaveLoader::Tests::DCOneSec);

    const char* testNorm = R"foo(
        <region>
        ampeg_release=.001
        sample=r1
    )foo";

    const char* testOneShot = R"foo(
        <region>
        sample=r1
        ampeg_release=.001
        loop_mode=one_shot
    )foo";

    const char* test = oneShot ? testOneShot : testNorm;

    SInstrumentPtr inst = std::make_shared<SInstrument>();
    auto perr = SParse::go(test, inst);
    assert(perr.empty());

    SamplerErrorContext cerr;
    auto ci = CompiledInstrument::make(cerr, inst);
    assert(ci);

    VoicePlayParameter params;
    params.midiPitch = 60;
    params.midiVelocity = 60;

    Sampler4vx samp;
    samp.setLoader(w);
    samp.setPatch(ci);

    VoicePlayInfo info;
    //  ci->play(info, params, w.get(), 44100.f);
    const int channel = 0;
    const int midiPitch = 60;
    const int midiVel = 60;
    samp.note_on(channel, midiPitch, midiVel, 44100);

    auto gates = SimdBlocks::maskTrue();
    const float sampleTime = 1.f / 44100.f;
    float_4 lfm = SimdBlocks::maskFalse();

    float largest = 0;
    for (int i = 0; i < 200; ++i) {
        auto x = samp.step(gates, sampleTime, lfm, false);
        //SQINFO("x = %f", x[0]);
        largest = std::max(x[0], largest);
    }
    SQINFO("larges was %f", largest);
    // while playing, should have something
    assert(largest > .2);

    // takes gates low, pump a little
    SQINFO("gates go low now");
    gates = SimdBlocks::maskFalse();
    for (int i = 0; i < 200; ++i) {
        auto x = samp.step(gates, sampleTime, lfm, false);
    }


    // now see if playing
    largest = 0;
    for (int i = 0; i < 200; ++i) {
        auto x = samp.step(gates, sampleTime, lfm, false);
        largest = std::max(x[0], largest);
    }
    SQINFO("largest was %f", largest);
    const bool wasSignal =  largest > .01f;
    assertEQ(wasSignal, oneShot);

}

void testx5() {
#if 0
    testSampler();
    testSampler4vxTestOutput();

    testSampler4vxRelease();
    testSampler4vxAttack();
    // no working any more
    //testSamplerRealSound();
    testSamplerRelease2();
    testSampQantizer();
    testSampBuilds();

    testSampPitch();
    testSampOffset();
#endif
    testOneShot(false);
    testOneShot(true);
}