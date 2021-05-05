
#include "Compressor.h"
#include "Compressor2.h"
#include "SqLog.h"
#include "asserts.h"
#include "tutil.h"

static void testLimiterPolyL() {
    using Comp = Compressor<TestComposite>;
    testPolyChannels<Comp>(Comp::LAUDIO_INPUT, Comp::LAUDIO_OUTPUT, 16);
}

static void testLimiterPolyR() {
    using Comp = Compressor<TestComposite>;
    testPolyChannels<Comp>(Comp::RAUDIO_INPUT, Comp::RAUDIO_OUTPUT, 16);
}

static void testCompUI() {
    using Comp = Compressor<TestComposite>;
    auto r = Comp::ratios();
    assert(r.size() == size_t(Cmprsr::Ratios::NUM_RATIOS));

    Cmprsr c;
    c.setIsPolyCV(false);
    for (int i = 0; i < int(Cmprsr::Ratios::NUM_RATIOS); ++i) {
        c.setCurve(Cmprsr::Ratios(i));
        c.step(0);
    }
}

static void testCompLim(int inputId, int outputId) {
    using Comp = Compressor<TestComposite>;
    std::shared_ptr<Comp> comp = std::make_shared<Comp>();
    initComposite(*comp);

    comp->params[Comp::RATIO_PARAM].value = float(int(Cmprsr::Ratios::HardLimit));
    comp->params[Comp::THRESHOLD_PARAM].value = .1f;
    const double threshV = Comp::getSlowThresholdFunction()(.1);
    //printf("th .1 give %f volts\n", threshV);

    comp->inputs[inputId].channels = 1;
    comp->outputs[outputId].channels = 1;

    // at threshold, should get thresh out.
    comp->inputs[inputId].setVoltage(float(threshV), 0);
    TestComposite::ProcessArgs args;
    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }

    float output = comp->outputs[outputId].voltages[0];
    assertClose(output, threshV, .01);

    comp->inputs[inputId].setVoltage(float(threshV), 0);
    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }
    output = comp->outputs[outputId].voltages[0];
    assertClose(output, threshV, .01);

    assertGT(4, threshV);
    comp->inputs[inputId].setVoltage(4, 0);
    for (int i = 0; i < 2000; ++i) {
        comp->process(args);
    }
    output = comp->outputs[outputId].voltages[0];
    assertClose(output, threshV, .01);

    comp->inputs[inputId].setVoltage(10, 0);
    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }
    output = comp->outputs[outputId].voltages[0];
    assertClose(output, threshV, .01);

    comp->inputs[inputId].setVoltage(0, 0);
    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }
    output = comp->outputs[outputId].voltages[0];
    assertClose(output, 0, .001);
}

static void testCompLim() {
    using Comp = Compressor<TestComposite>;
    testCompLim(Comp::LAUDIO_INPUT, Comp::LAUDIO_OUTPUT);
    testCompLim(Comp::RAUDIO_INPUT, Comp::RAUDIO_OUTPUT);
}

static void testCompRatio(int inputId, int outputId, Cmprsr::Ratios ratio) {
    using Comp = Compressor<TestComposite>;
    std::shared_ptr<Comp> comp = std::make_shared<Comp>();
    initComposite(*comp);

    comp->params[Comp::RATIO_PARAM].value = float(int(ratio));
    comp->params[Comp::THRESHOLD_PARAM].value = .1f;
    const double threshV = Comp::getSlowThresholdFunction()(.1);

    comp->inputs[inputId].channels = 1;
    comp->outputs[outputId].channels = 1;

    // at threshold, should get thresh out.
    comp->inputs[inputId].setVoltage(float(threshV), 0);
    TestComposite::ProcessArgs args;
    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }

    float output = comp->outputs[outputId].voltages[0];
    assertClose(output, threshV, .01);

    const float threshDb = float(AudioMath::db(threshV));

    float expectedRatio = 0;
    switch (ratio) {
        case Cmprsr::Ratios::_2_1_hard:
            expectedRatio = 2;
            break;
        case Cmprsr::Ratios::_4_1_hard:
            expectedRatio = 4;
            break;
        case Cmprsr::Ratios::_8_1_hard:
            expectedRatio = 8;
            break;
        case Cmprsr::Ratios::_20_1_hard:
            expectedRatio = 20;
            break;
        default:
            assert(false);
    }

    for (int mult = 2; (mult * threshV) < 10; mult *= 2) {
        float input = float(threshV * mult);
        const float inputDb = float(AudioMath::db(input));
        comp->inputs[inputId].setVoltage(input, 0);
        for (int i = 0; i < 2000; ++i) {
            comp->process(args);
        }
        output = comp->outputs[outputId].voltages[0];
        float outputDb = float(AudioMath::db(output));

        const float observedRatio = (inputDb - threshDb) / (outputDb - threshDb);
        assertClosePct(observedRatio, expectedRatio, 15);
    }
}

static void testCompRatio8() {
    using Comp = Compressor<TestComposite>;
    testCompRatio(Comp::LAUDIO_INPUT, Comp::LAUDIO_OUTPUT, Cmprsr::Ratios::_8_1_hard);
    testCompRatio(Comp::LAUDIO_INPUT, Comp::LAUDIO_OUTPUT, Cmprsr::Ratios::_4_1_hard);
    testCompRatio(Comp::LAUDIO_INPUT, Comp::LAUDIO_OUTPUT, Cmprsr::Ratios::_20_1_hard);
    testCompRatio(Comp::LAUDIO_INPUT, Comp::LAUDIO_OUTPUT, Cmprsr::Ratios::_2_1_hard);
}

template <class T>
class TestBothComp {
public:
    TestBothComp() {
        comp_ = std::make_shared<T>();
        initComposite(*comp_);
    }

    void testPoly() {
        // initial run with 1 channel
        setNumChan(1);
        setInputs(1, 100.f);
        run(1000);
        const float x = comp_->outputs[T::LAUDIO_OUTPUT].voltages[0];

        assertLT(x, 50);
        assertGT(x, 1);

        // now patch more channels, see if comp recognizes the change
        setNumChan(4);
        setInputs(4, 100.f);
        run(1000);
        const float x0 = comp_->outputs[T::LAUDIO_OUTPUT].voltages[0];
        const float x1 = comp_->outputs[T::LAUDIO_OUTPUT].voltages[1];
        const float x2 = comp_->outputs[T::LAUDIO_OUTPUT].voltages[2];
        const float x3 = comp_->outputs[T::LAUDIO_OUTPUT].voltages[3];
        assertLT(x0, 50);
        assertLT(x1, 50);
        assertLT(x2, 50);
        assertLT(x3, 50);
        assertGT(x0, 1);
        assertGT(x1, 1);
        assertGT(x2, 1);
        assertGT(x3, 1);
    }

private:
    std::shared_ptr<T> comp_;
    TestComposite::ProcessArgs args;

    void setNumChan(int x) {
        comp_->inputs[T::LAUDIO_INPUT].channels = x;
        comp_->outputs[T::LAUDIO_OUTPUT].channels = x;
    }
    void run(int iterations) {
        for (int i = 0; i < iterations; ++i) {
            comp_->process(args);
        }
    }

    void setInputs(int num, float value) {
        for (int i = 0; i < num; ++i) {
            comp_->inputs[T::LAUDIO_INPUT].setVoltage(value, i);
        }
    }
};

static void testCompPoly() {
    TestBothComp<Compressor2<TestComposite>> test2;
    test2.testPoly();

    TestBothComp<Compressor<TestComposite>> test;
    test.testPoly();
}

static void testCompPolyOrig() {
    using Comp = Compressor<TestComposite>;
    std::shared_ptr<Comp> comp = std::make_shared<Comp>();
    initComposite(*comp);

    comp->inputs[Comp::LAUDIO_INPUT].channels = 1;
    comp->outputs[Comp::LAUDIO_OUTPUT].channels = 1;

    // huge input.
    comp->inputs[Comp::LAUDIO_INPUT].setVoltage(100, 0);
    TestComposite::ProcessArgs args;
    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }
    float x = comp->outputs[Comp::LAUDIO_OUTPUT].voltages[0];
    assertLT(x, 50);

    const int nchTest = 4;
    comp->inputs[Comp::LAUDIO_INPUT].channels = nchTest;
    comp->outputs[Comp::LAUDIO_OUTPUT].channels = nchTest;

    for (int i = 0; i < nchTest; ++i) {
        comp->inputs[Comp::LAUDIO_INPUT].setVoltage(100, i);
    }

    for (int i = 0; i < 1000; ++i) {
        comp->process(args);
    }
    float x0 = comp->outputs[Comp::LAUDIO_OUTPUT].voltages[0];
    float x1 = comp->outputs[Comp::LAUDIO_OUTPUT].voltages[1];
    float x2 = comp->outputs[Comp::LAUDIO_OUTPUT].voltages[2];
    float x3 = comp->outputs[Comp::LAUDIO_OUTPUT].voltages[3];
    assertLT(x0, 50);
    assertLT(x1, 50);
    assertLT(x2, 50);
    assertLT(x3, 50);
}

using Comp2 = Compressor2<TestComposite>;
static void run(Comp2& comp, int times) {
    TestComposite::ProcessArgs args;
    for (int i = 0; i < times; ++i) {
        comp.process(args);
    }
}

static void init(Comp2& comp) {
    comp.init();
    initComposite(comp);
}

static void testPolyInit() {
    Comp2 comp;

    // before anything, params should all be zero
    for (int i = 0; i < Comp2::NUM_PARAMS; ++i) {
        assertEQ(comp.params[i].value, 0.f);
    }

    init(comp);

    {
        const CompressorParmHolder& holder = comp.getParamHolder();
        auto def = getDefaultParamValues<Comp2>();
        for (int i = 0; i < def.size(); ++i) {
            assertEQ(comp.params[i].value, def[i]);
        }
        for (int i = 0; i < 16; ++i) {
            assertEQ(holder.getAttack(i), def[Comp2::ATTACK_PARAM]);
            assertEQ(holder.getRelease(i), def[Comp2::RELEASE_PARAM]);
            assertEQ(holder.getThreshold(i), def[Comp2::THRESHOLD_PARAM]);
            assertEQ(holder.getRatio(i), def[Comp2::RATIO_PARAM]);
            assertEQ(holder.getMakeupGain(i), def[Comp2::MAKEUPGAIN_PARAM]);
            assertEQ(holder.getEnabled(i), bool(std::round(def[Comp2::NOTBYPASS_PARAM])));
            assertEQ(holder.getWetDryMix(i), def[Comp2::WETDRY_PARAM]);
        }
    }
    run(comp, 40);

    auto& holder = comp.getParamHolder();
    float a = holder.getAttack(0);
    float r = holder.getRelease(0);

    for (int channel = 0; channel < 16; ++channel) {
        assertEQ(holder.getAttack(channel), a);
        assertEQ(holder.getRelease(channel), r);
    }
}

static void testPolyAttack(int channel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int bank = channel / 4;
    const int subChannel = channel % 4;

    // assert that all are the same to starts
    if (channel == 0) {
        simd_assertEQ(comp._getComp(0)._getAF()._getL(), comp._getComp(1)._getAF()._getL());
        simd_assertEQ(comp._getComp(0)._getAF()._getL(), comp._getComp(2)._getAF()._getL());
        simd_assertEQ(comp._getComp(0)._getAF()._getL(), comp._getComp(3)._getAF()._getL());

        const float_4 x = comp._getComp(0)._getAF()._getL();
        assertEQ(x[0], x[1]);
        assertEQ(x[0], x[2]);
        assertEQ(x[0], x[3]);
    }

    // get the four channel compressor. assert on initial conditions
    Cmprsr& c = comp._getComp(bank);
    const MultiLPF2& lpf = c._getAF();

    comp.params[Comp2::CHANNEL_PARAM].value = channel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::ATTACK_PARAM].value = .2f;
    run(comp, 40);

    const float_4 af = lpf._getL();
    for (int i = 0; i < 4; ++i) {
        int other = (i + 1) % 4;
        if (i == subChannel) {
            assertNE(af[i], af[other])
        } else if (other != subChannel) {
            // figure out later
            //   assertEQ(af[i], af[other]);
        }
    }
}

static void testPolyStereoAttack(int stereoChannel) {
    assert(stereoChannel >= 0 && stereoChannel < 8);

    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int leftChannel = stereoChannel * 2;
    const int rightChannel = leftChannel + 1;
    const int bank = leftChannel / 4;
    assert(bank == (rightChannel / 4));
    const int leftSubChannel = stereoChannel % 4;

    // get the four channel compressor. assert on initial conditions
    Cmprsr& c = comp._getComp(bank);
    const MultiLPF2& lpf = c._getAF();

    comp.params[Comp2::STEREO_PARAM].value = 1;
    comp.params[Comp2::CHANNEL_PARAM].value = stereoChannel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::ATTACK_PARAM].value = .2f;
    run(comp, 40);

    const float_4 af = lpf._getL();

    // stereo pairs
    assertEQ(af[0], af[1]);
    assertEQ(af[2], af[3]);
    assertNE(af[0], af[2]);
}

static void testPolyRelease(int channel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int bank = channel / 4;
    const int subChannel = channel % 4;

    // assert that all are the same to starts
    if (channel == 0) {
        simd_assertEQ(comp._getComp(0)._getLag()._getLRelease(), comp._getComp(1)._getLag()._getLRelease());
        simd_assertEQ(comp._getComp(0)._getLag()._getLRelease(), comp._getComp(2)._getLag()._getLRelease());
        simd_assertEQ(comp._getComp(0)._getLag()._getLRelease(), comp._getComp(3)._getLag()._getLRelease());
        const float_4 x = comp._getComp(0)._getLag()._getLRelease();
        assertEQ(x[0], x[1]);
        assertEQ(x[0], x[2]);
        assertEQ(x[0], x[3]);
    }

    // get the four channel compressor. assert on initial conditions
    Cmprsr& c = comp._getComp(bank);
    const MultiLag2& lag = c._getLag();

    comp.params[Comp2::CHANNEL_PARAM].value = channel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::RELEASE_PARAM].value = .2f;
    run(comp, 40);

    const float_4 rf = lag._getLRelease();
    for (int i = 0; i < 4; ++i) {
        int other = (i + 1) % 4;
        if (i == subChannel) {
            assertNE(rf[i], rf[other])
        } else if (other != subChannel) {
            // figure out later
            //   assertEQ(af[i], af[other]);
        }
    }
}

static void testPolyStereoRelease(int stereoChannel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int leftChannel = stereoChannel * 2;
    const int bank = leftChannel / 4;

    // get the four channel compressor. assert on initial conditions
    Cmprsr& c = comp._getComp(bank);
    const MultiLag2& lag = c._getLag();

    comp.params[Comp2::STEREO_PARAM].value = 1;
    comp.params[Comp2::CHANNEL_PARAM].value = stereoChannel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::RELEASE_PARAM].value = .2f;
    run(comp, 40);

    const float_4 rf = lag._getLRelease();

    // stereo pairs
    assertEQ(rf[0], rf[1]);
    assertEQ(rf[2], rf[3]);
    assertNE(rf[0], rf[2]);
}

static void testPolyThreshold(int channel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int bank = channel / 4;
    const int subChannel = channel % 4;

    // assert that all are the same to starts
    if (channel == 0) {
        simd_assertEQ(comp._getComp(0)._getTh(), comp._getComp(1)._getTh());
        simd_assertEQ(comp._getComp(0)._getTh(), comp._getComp(2)._getTh());
        simd_assertEQ(comp._getComp(0)._getTh(), comp._getComp(3)._getTh());

        const float_4 x = comp._getComp(0)._getTh();
        assertEQ(x[0], x[1]);
        assertEQ(x[0], x[2]);
        assertEQ(x[0], x[3]);
    }

    // get the four channel compressor. assert on initial conditions
    Cmprsr& c = comp._getComp(bank);

    comp.params[Comp2::CHANNEL_PARAM].value = channel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::THRESHOLD_PARAM].value = .2f;
    run(comp, 40);

    const float_4 th = c._getTh();
    for (int i = 0; i < 4; ++i) {
        int other = (i + 1) % 4;
        if (i == subChannel) {
            assertNE(th[i], th[other])
        } else if (other != subChannel) {
            // figure out later
            //   assertEQ(af[i], af[other]);
        }
    }
}

static void testPolyStereoThreshold(int stereoChannel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int leftChannel = stereoChannel * 2;
    const int bank = leftChannel / 4;

    comp.params[Comp2::STEREO_PARAM].value = 1;
    comp.params[Comp2::CHANNEL_PARAM].value = stereoChannel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::THRESHOLD_PARAM].value = .2f;
    run(comp, 40);

    Cmprsr& c = comp._getComp(bank);
    const float_4 th = c._getTh();
    // stereo pairs
    assertEQ(th[0], th[1]);
    assertEQ(th[2], th[3]);
    assertNE(th[0], th[2]);
}

static void testPolyRatio(int channel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int bank = channel / 4;
    const int subChannel = channel % 4;

    // assert that all are the same to starts
    if (channel == 0) {
        for (int i = 0; i < 4; ++i) {
            assertEQ(int(comp._getComp(0)._getRatio()[i]), int(comp._getComp(1)._getRatio()[i]));
            assertEQ(int(comp._getComp(0)._getRatio()[i]), int(comp._getComp(2)._getRatio()[i]));
            assertEQ(int(comp._getComp(0)._getRatio()[i]), int(comp._getComp(3)._getRatio()[i]));
        }

        auto x = comp._getComp(0)._getRatio();
        assertEQ(int(x[0]), int(x[1]));
        assertEQ(int(x[0]), int(x[2]));
        assertEQ(int(x[0]), int(x[3]));
    }

    comp.params[Comp2::CHANNEL_PARAM].value = channel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::RATIO_PARAM].value = .2f;
    run(comp, 40);

    auto r = comp._getComp(bank)._getRatio();
    for (int i = 0; i < 4; ++i) {
        int other = (i + 1) % 4;
        if (i == subChannel) {
            assertNE(int(r[i]), int(r[other]))
        } else if (other != subChannel) {
            // figure out later
            //   assertEQ(af[i], af[other]);
        }
    }
}

static void testPolyStereoRatio(int stereoChannel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int leftChannel = stereoChannel * 2;
    const int bank = leftChannel / 4;

    comp.params[Comp2::STEREO_PARAM].value = 1;
    comp.params[Comp2::CHANNEL_PARAM].value = stereoChannel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::RATIO_PARAM].value = .2f;
    run(comp, 40);

    auto r = comp._getComp(bank)._getRatio();
    // stereo pairs
    assertEQ(int(r[0]), int(r[1]));
    assertEQ(int(r[2]), int(r[3]));
    assertNE(int(r[0]), int(r[2]));
}

static void testPolyWetDry(int channel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int bank = channel / 4;
    const int subChannel = channel % 4;

    // assert that all are the same to starts
    if (channel == 0) {
        for (int i = 0; i < 4; ++i) {
            assertEQ(int(comp._getComp(0)._getRatio()[i]), int(comp._getComp(1)._getRatio()[i]));
            assertEQ(int(comp._getComp(0)._getRatio()[i]), int(comp._getComp(2)._getRatio()[i]));
            assertEQ(int(comp._getComp(0)._getRatio()[i]), int(comp._getComp(3)._getRatio()[i]));
        }

        auto x = comp._getComp(0)._getRatio();
        assertEQ(int(x[0]), int(x[1]));
        assertEQ(int(x[0]), int(x[2]));
        assertEQ(int(x[0]), int(x[3]));
    }

    comp.params[Comp2::CHANNEL_PARAM].value = channel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::WETDRY_PARAM].value = .2f;
    run(comp, 40);

    auto w = comp._getWet(bank);
    for (int i = 0; i < 4; ++i) {
        int other = (i + 1) % 4;
        if (i == subChannel) {
            assertNE(w[i], w[other])
        } else if (other != subChannel) {
            // figure out later
            //   assertEQ(af[i], af[other]);
        }
    }
}

static void testPolyStereoWetDry(int stereoChannel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int leftChannel = stereoChannel * 2;
    const int bank = leftChannel / 4;

    comp.params[Comp2::STEREO_PARAM].value = 1;
    comp.params[Comp2::CHANNEL_PARAM].value = stereoChannel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::WETDRY_PARAM].value = .2f;
    run(comp, 40);

    auto w = comp._getWet(bank);
    // stereo pairs
    assertEQ(w[0], w[1]);
    assertEQ(w[2], w[3]);
    assertNE(w[0], w[2]);
}

static void testBypass(int channel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int bank = channel / 4;
    const int subChannel = channel % 4;

    // assert that all are the same to starts
    if (channel == 0) {
        float_4 x0 = SimdBlocks::ifelse(comp._getEn(0), float_4(1), float_4(0));
        float_4 x1 = SimdBlocks::ifelse(comp._getEn(1), float_4(1), float_4(0));
        float_4 x2 = SimdBlocks::ifelse(comp._getEn(2), float_4(1), float_4(0));
        float_4 x3 = SimdBlocks::ifelse(comp._getEn(3), float_4(1), float_4(0));
        simd_assertEQ(x0, x1);
        simd_assertEQ(x0, x2);
        simd_assertEQ(x0, x3);
    }

    comp.params[Comp2::CHANNEL_PARAM].value = channel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::NOTBYPASS_PARAM].value = 0.f;
    run(comp, 40);

    auto e = comp._getEn(bank);
    float_4 en = SimdBlocks::ifelse(e, float_4(1), float_4(0));
    for (int i = 0; i < 4; ++i) {
        int other = (i + 1) % 4;
        if (i == subChannel) {
            assertNE(en[i], en[other])
        } else if (other != subChannel) {
            // figure out later
            //   assertEQ(af[i], af[other]);
        }
    }
}

static void testStereoBypass(int stereoChannel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int leftChannel = stereoChannel * 2;
    const int bank = leftChannel / 4;

    comp.params[Comp2::STEREO_PARAM].value = 1;
    comp.params[Comp2::CHANNEL_PARAM].value = stereoChannel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::NOTBYPASS_PARAM].value = .0f;
    run(comp, 40);

    auto e = comp._getEn(bank);
    float_4 en = SimdBlocks::ifelse(e, float_4(1), float_4(0));
    // stereo pairs
    assertEQ(en[0], en[1]);
    assertEQ(en[2], en[3]);
    assertNE(en[0], en[2]);
}

static void testGain(int channel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int bank = channel / 4;
    const int subChannel = channel % 4;

    // assert that all are the same to starts
    if (channel == 0) {
        float_4 x0 = comp._getG(0);
        float_4 x1 = comp._getG(1);
        float_4 x2 = comp._getG(2);
        float_4 x3 = comp._getG(3);
        simd_assertEQ(x0, x1);
        simd_assertEQ(x0, x2);
        simd_assertEQ(x0, x3);

        assertEQ(x0[0], x0[1]);
        assertEQ(x0[0], x0[2]);
        assertEQ(x0[0], x0[3]);
    }

    comp.params[Comp2::CHANNEL_PARAM].value = channel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::MAKEUPGAIN_PARAM].value = 0.5f;
    run(comp, 40);

    auto g = comp._getG(bank);
    for (int i = 0; i < 4; ++i) {
        int other = (i + 1) % 4;
        if (i == subChannel) {
            assertNE(g[i], g[other])
        } else if (other != subChannel) {
            // figure out later
            //   assertEQ(af[i], af[other]);
        }
    }
}

static void testStereoGain(int stereoChannel) {
    Comp2 comp;
    init(comp);
    run(comp, 40);

    const int leftChannel = stereoChannel * 2;
    const int bank = leftChannel / 4;

    comp.params[Comp2::STEREO_PARAM].value = 1;
    comp.params[Comp2::CHANNEL_PARAM].value = stereoChannel + 1.f;  // offset 01
    run(comp, 40);
    comp.params[Comp2::MAKEUPGAIN_PARAM].value = .2f;
    run(comp, 40);

    auto g = comp._getG(bank);
    // stereo pairs
    assertEQ(g[0], g[1]);
    assertEQ(g[2], g[3]);
    assertNE(g[0], g[2]);
}

static void testPolyParams() {
    for (int i = 0; i < 15; ++i) {
        testPolyAttack(i);
        testPolyRelease(i);
        testPolyThreshold(i);
        testPolyRatio(i);
        testPolyWetDry(i);
        testBypass(i);
        testGain(i);
    }
}

static void testPolyStereoParams() {
    for (int i = 0; i < 8; ++i) {
        testPolyStereoAttack(i);
        testPolyStereoRelease(i);
        testPolyStereoThreshold(i);
        testPolyStereoRatio(i);
        testPolyStereoWetDry(i);
        testStereoBypass(i);
        testStereoGain(i);
    }
}

void testCompressor() {
    testLimiterPolyL();
    testLimiterPolyR();

    testCompUI();
    testCompLim();
    testCompRatio8();

    // testCompPolyOrig();
    testCompPoly();
    testPolyInit();
    testPolyParams();
    testPolyStereoParams();
}