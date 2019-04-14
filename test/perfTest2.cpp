
#include "LookupTable.h"
#include "Mix8.h"
#include "Mix4.h"
#include "MixM.h"
#include "MultiLag.h"
#include "ObjectCache.h"
#include "Slew4.h"
#include "TestComposite.h"

#include "MeasureTime.h"

extern double overheadOutOnly;
extern double overheadInOut;

static void testMultiLPF()
{
    MultiLPF<8> lpf;

    lpf.setCutoff(.01f);
    float input[8];

    MeasureTime<float>::run(overheadInOut, "multi lpf", [&lpf, &input]() {
        float x = TestBuffers<float>::get();
        for (int i = 0; i < 8; ++i) {
            input[i] = x;
        }
        lpf.step(input);

        return lpf.get(3);
        }, 1);
}

static void testMultiLag()
{
    MultiLag<8> lpf;

    lpf.setAttack(.01f);
    lpf.setRelease(.02f);
    float input[8];

    MeasureTime<float>::run(overheadInOut, "multi lag", [&lpf, &input]() {
        float x = TestBuffers<float>::get();
        for (int i = 0; i < 8; ++i) {
            input[i] = x;
        }
        lpf.step(input);

        return lpf.get(3);
        }, 1);
}


static void testMultiLPFMod()
{
    MultiLPF<8> lpf;

    lpf.setCutoff(.01f);
    float input[8];

    MeasureTime<float>::run(overheadInOut, "multi lpf mod", [&lpf, &input]() {

        float x = TestBuffers<float>::get();
        float y = TestBuffers<float>::get();
        y = std::abs(y) + .1f;
        while (y > .49f) {
            y -= .48f;
        }
        assert(y > 0);
        assert(y < .5);

        lpf.setCutoff(y);
        for (int i = 0; i < 8; ++i) {
            input[i] = x;
        }
        lpf.step(input);

        return lpf.get(3);
        }, 1);
}


static void testMultiLagMod()
{
    MultiLag<8> lag;

    lag.setAttack(.01f);
    lag.setRelease(.02f);
    float input[8];

    MeasureTime<float>::run(overheadInOut, "multi lag mod", [&lag, &input]() {

        float x = TestBuffers<float>::get();
        float y = TestBuffers<float>::get();
        y = std::abs(y) + .1f;
        while (y > .49f) {
            y -= .48f;
        }
        assert(y > 0);
        assert(y < .5);

        lag.setAttack(y);
        lag.setRelease(y/2);
        for (int i = 0; i < 8; ++i) {
            input[i] = x;
        }
        lag.step(input);

        return lag.get(3);
        }, 1);
}

static void testUniformLookup()
{
    std::shared_ptr<LookupTableParams<float>> lookup = ObjectCache<float>::getSinLookup();
    MeasureTime<float>::run(overheadInOut, "uniform", [lookup]() {
        float x = TestBuffers<float>::get();
        return LookupTable<float>::lookup(*lookup, x, true);

    }, 1);
}
static void testNonUniform()
{
    std::shared_ptr<NonUniformLookupTableParams<float>> lookup = makeLPFilterL_Lookup<float>();

    MeasureTime<float>::run(overheadInOut, "non-uniform", [lookup]() {
        float x = TestBuffers<float>::get();
        return  NonUniformLookupTable<float>::lookup(*lookup, x);
     
    }, 1);
}

using Slewer = Slew4<TestComposite>;

static void testSlew4()
{
    Slewer fs;

    fs.init();

    fs.inputs[Slewer::INPUT_AUDIO0].value = 0;

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "Slade", [&fs]() {
        fs.inputs[Slewer::INPUT_TRIGGER0].value = TestBuffers<float>::get();
        fs.step();
        return fs.outputs[Slewer::OUTPUT0].value;
        }, 1);
}

using Mixer8 = Mix8<TestComposite>;
static void testMix8()
{
    Mixer8 fs;

    fs.init();

    fs.inputs[fs.AUDIO0_INPUT].value = 0;

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "mix8", [&fs]() {
        fs.inputs[Slewer::INPUT_TRIGGER0].value = TestBuffers<float>::get();
        fs.step();
        return fs.outputs[Slewer::OUTPUT0].value;
      }, 1);
}

using Mixer4 = Mix4<TestComposite>;
static void testMix4()
{
    Mixer4 fs;
    fs.init();
    fs.inputs[fs.AUDIO0_INPUT].value = 0;

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "mix4", [&fs]() {
        fs.inputs[Slewer::INPUT_TRIGGER0].value = TestBuffers<float>::get();
        fs.step();
        return fs.outputs[Slewer::OUTPUT0].value;
        }, 1);
}

using MixerM = MixM<TestComposite>;
static void testMixM()
{
    MixerM fs;

    fs.init();

    fs.inputs[fs.AUDIO0_INPUT].value = 0;

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "mixM", [&fs]() {
        fs.inputs[Slewer::INPUT_TRIGGER0].value = TestBuffers<float>::get();
        fs.step();
        return fs.outputs[Slewer::OUTPUT0].value;
        }, 1);
}
void perfTest2()
{
    testSlew4();
    testMix8();
    testMix4();
    testMixM();
    testUniformLookup();
    testNonUniform();
    testMultiLPF();
    testMultiLPFMod();
    testMultiLag();
    testMultiLagMod();

}