#include "TestComposite.h"

#include "DrumTrigger.h"

#include "Filt.h"

#include "LookupTable.h"
#include "Mix8.h"
#include "Mix4.h"
#include "MixM.h"
#include "MixStereo.h"

#ifndef _MSC_VER
#include "MultiLag.h"
#include "F2_Poly.h"
#include "Compressor.h"
#endif

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
    printf("non uniform lookup size = %d\n", lookup->size()); 

    MeasureTime<float>::run(overheadInOut, "non-uniform", [lookup]() {
        float x = TestBuffers<float>::get();
        return  NonUniformLookupTable<float>::lookup(*lookup, x);
     
    }, 1);
    printf("Now abort");
    abort();
}

using Slewer = Slew4<TestComposite>;

static void testSlew4()
{
    Slewer fs;

    fs.init();

    fs.inputs[Slewer::INPUT_AUDIO0].setVoltage(0, 0);

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "Slade", [&fs]() {
        fs.inputs[Slewer::INPUT_TRIGGER0].setVoltage(TestBuffers<float>::get(), 0);
        fs.step();
        return fs.outputs[Slewer::OUTPUT0].getVoltage(0);
        }, 1);
}

using DT = DrumTrigger<TestComposite>;
static void testDrumTrigger()
{
    DT fs;
    fs.init();
    fs.inputs[DT::CV_INPUT].channels = 1;
    fs.inputs[DT::CV_INPUT].channels = 8;
    fs.outputs[DT::GATE0_OUTPUT].channels = 1;
    fs.outputs[DT::GATE0_OUTPUT].channels = 8;
    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "Polygate", [&fs]() {
        fs.inputs[DT::CV_INPUT].setVoltage(TestBuffers<float>::get(), 0);
        fs.step();
        return fs.outputs[DT::GATE0_OUTPUT].getVoltage(0);
        }, 1);
}

using Filter = Filt<TestComposite>;
static void testFilt()
{
    Filter fs;
    fs.init();
    fs.inputs[Filter::L_AUDIO_INPUT].channels = 1;
    fs.outputs[Filter::L_AUDIO_OUTPUT].channels = 1;
    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "filt", [&fs]() {
        fs.inputs[Filter::L_AUDIO_INPUT].setVoltage(TestBuffers<float>::get(), 0);
        fs.step();
        return fs.outputs[Filter::L_AUDIO_OUTPUT].getVoltage(0);
        }, 1);
}

static void testFilt2()
{
    Filter fs;
    fs.init();
    fs.inputs[Filter::L_AUDIO_INPUT].channels = 1;
    fs.outputs[Filter::L_AUDIO_OUTPUT].channels = 1;
    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "filt w/mod", [&fs]() {
        fs.inputs[Filter::L_AUDIO_INPUT].setVoltage(TestBuffers<float>::get(), 0);
        fs.params[Filter::FC_PARAM].value = TestBuffers<float>::get();
        fs.step();
        return fs.outputs[Filter::L_AUDIO_OUTPUT].getVoltage(0);
     }, 1);
}

using Mixer8 = Mix8<TestComposite>;
static void testMix8()
{
    Mixer8 fs;

    fs.init();

    fs.inputs[fs.AUDIO0_INPUT].setVoltage(0, 0);

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "mix8", [&fs]() {
        fs.inputs[Slewer::INPUT_TRIGGER0].setVoltage(TestBuffers<float>::get(), 0);
        fs.step();
        return fs.outputs[Slewer::OUTPUT0].getVoltage(0);
      }, 1);
}

using Mixer4 = Mix4<TestComposite>;
static void testMix4()
{
    Mixer4 fs;
    fs.init();
    fs.inputs[fs.AUDIO0_INPUT].setVoltage(0, 0);

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "mix4", [&fs]() {
        fs.inputs[Slewer::INPUT_TRIGGER0].setVoltage(TestBuffers<float>::get(), 0);
        fs.step();
        return fs.outputs[Slewer::OUTPUT0].getVoltage(0);
        }, 1);
}

using MixerSt = MixStereo<TestComposite>;
static void testMixStereo()
{
    MixerSt fs;
    fs.init();
    fs.inputs[fs.AUDIO0_INPUT].setVoltage(0, 0);

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "mix stereo", [&fs]() {
        fs.inputs[Slewer::INPUT_TRIGGER0].setVoltage(TestBuffers<float>::get(), 0);
        fs.step();
        return fs.outputs[Slewer::OUTPUT0].getVoltage(0);
        }, 1);
}

using MixerM = MixM<TestComposite>;
static void testMixM()
{
    MixerM fs;

    fs.init();

    fs.inputs[fs.AUDIO0_INPUT].setVoltage(0, 0);

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(overheadInOut, "mixM", [&fs]() {
        fs.inputs[Slewer::INPUT_TRIGGER0].setVoltage(TestBuffers<float>::get());
        fs.step();
        return fs.outputs[Slewer::OUTPUT0].getVoltage(0);
        }, 1);
}


#ifndef _MSC_VER

static void testF2_Poly1()
{
    using Comp = F2_Poly<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::AUDIO_INPUT].setVoltage(0, 0);
    comp.inputs[Comp::AUDIO_INPUT].channels = 1;
 
    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadInOut, "testF2 (new) mono 12/Lim", [&comp, args]() {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(TestBuffers<float>::get());
        comp.process(args);
        return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        }, 1);
}

static void testF2_Poly16()
{
    using Comp = F2_Poly<TestComposite>;
    Comp comp;

    comp.init();

    
    comp.inputs[Comp::AUDIO_INPUT].channels = 16;
    for (int i=0; i<16; ++i) {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(0, i);
    }
 
    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadInOut, "testF2 (new) 16 ch", [&comp, args]() {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(TestBuffers<float>::get());
        comp.process(args);
        return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        }, 1);
}

static void testF2_12nl()
{
    using Comp = F2_Poly<TestComposite>;
    Comp comp;

    comp.init();
    
    comp.inputs[Comp::AUDIO_INPUT].channels = 1;
    comp.params[Comp::TOPOLOGY_PARAM].value = float(Comp::Topology::SINGLE);
    comp.params[Comp::LIMITER_PARAM].value = 0;
    for (int i=0; i<16; ++i) {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(0, i);
    }
 
    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadInOut, "testF2 (12/nl)", [&comp, args]() {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(TestBuffers<float>::get());
        comp.process(args);
        return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        }, 1);
}

static void testF2_24l()
{
    using Comp = F2_Poly<TestComposite>;
    Comp comp;

    comp.init();
    
    comp.inputs[Comp::AUDIO_INPUT].channels = 1;
    comp.params[Comp::TOPOLOGY_PARAM].value = float(Comp::Topology::SERIES);
    comp.params[Comp::LIMITER_PARAM].value = 1;
    for (int i=0; i<16; ++i) {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(0, i);
    }
 
    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadInOut, "testF2 (24/lim)", [&comp, args]() {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(TestBuffers<float>::get());
        comp.process(args);
        return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        }, 1);
}

static void testCompLim1()
{
    using Comp = Compressor<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::LAUDIO_INPUT].channels = 1;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 0;      // limiter
    comp.params[Comp::NOTBYPASS_PARAM].value = 1;


    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;


    MeasureTime<float>::run(overheadInOut, "Comp/Lim 1 channel", [&comp, args]() {
        comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
        comp.process(args);
        return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        }, 1);
}

static void testCompLim16()
{
   using Comp = Compressor<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::LAUDIO_INPUT].channels = 16;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 0;      // limiter
    comp.params[Comp::NOTBYPASS_PARAM].value = 1;

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadInOut, "Comp/Lim 16 channel", [&comp, args]() {
        comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
        comp.process(args);
        return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        }, 1);
}

#if 0
static void testCompLim16Dist()
{
   using Comp = Compressor<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::LAUDIO_INPUT].channels = 16;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 0;      // limiter
   // comp.params[Comp::REDUCEDISTORTION_PARAM].value = 1;  

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadInOut, "Comp/Lim 16 ndist channel", [&comp, args]() {
        comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
        comp.process(args);
        return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        }, 1);
}
#endif

static void testCompKnee()
{
    using Comp = Compressor<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::LAUDIO_INPUT].channels = 1;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 3;      // 4:1 hard knee
    comp.params[Comp::NOTBYPASS_PARAM].value = 1;


    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;


    MeasureTime<float>::run(overheadInOut, "Comp / Lim 1 channel 4:1 soft", [&comp, args]() {
        comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
        comp.process(args);
        return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        }, 1);
}

static void testCompKnee16()
{
    using Comp = Compressor<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::LAUDIO_INPUT].channels = 16;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 3;      // 4:1 sort knee
    comp.params[Comp::NOTBYPASS_PARAM].value = 1;
    printf("setting ratio to 1\n");


    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadInOut, "Comp / Lim 16 channel 4:1 soft", [&comp, args]() {
        comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
        comp.process(args);
        return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        }, 1);
}

static void testCompKnee16Hard()
{
    using Comp = Compressor<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::LAUDIO_INPUT].channels = 16;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 2;      // 4:1 hard knee
    comp.params[Comp::NOTBYPASS_PARAM].value = 1;


    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44199;

    MeasureTime<float>::run(overheadInOut, "Comp / Lim 16 channel 4:1 hard", [&comp, args]() {
        comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
        comp.process(args);
        return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        }, 1);
}

#endif


void perfTest2()
{
    assert(overheadInOut > 0);
    assert(overheadOutOnly > 0);
#ifndef _MSC_VER
   
    testCompLim1();
    testCompLim16();
    testCompKnee();
    testCompKnee16();
    testCompKnee16Hard();
    testF2_Poly1();
    testF2_Poly16();
    testF2_12nl();
    testF2_24l();
#endif


    testDrumTrigger();
    testFilt();
    testFilt2();
    testSlew4();
    testMixStereo();
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