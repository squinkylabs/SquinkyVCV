

#include "Compressor.h"
#include "Compressor2.h"

#include "tutil.h"

#include "asserts.h"


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



void testCompressor()
{
    testLimiterPolyL();
    testLimiterPolyR();

    testCompUI();
    testCompLim();

    testCompRatio8();

    // testCompPolyOrig();
    testCompPoly();
}