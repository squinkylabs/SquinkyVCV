#include "Compressor.h"
#include "Compressor2.h"
#include "DrumTrigger.h"
#include "F2_Poly.h"
#include "Filt.h"
#include "LookupTable.h"
#include "MeasureTime.h"
#include "Mix4.h"
#include "Mix8.h"
#include "MixM.h"
#include "MixStereo.h"
#include "MultiLag.h"
#include "ObjectCache.h"
#include "Slew4.h"
#include "TestComposite.h"
#include "tutil.h"

extern double overheadOutOnly;
extern double overheadInOut;

static void testMultiLPF() {
    MultiLPF<8> lpf;

    lpf.setCutoff(.01f);
    float input[8];

    MeasureTime<float>::run(
        overheadInOut, "multi lpf", [&lpf, &input]() {
            float x = TestBuffers<float>::get();
            for (int i = 0; i < 8; ++i) {
                input[i] = x;
            }
            lpf.step(input);

            return lpf.get(3);
        },
        1);
}

static void testMultiLag() {
    MultiLag<8> lpf;

    lpf.setAttack(.01f);
    lpf.setRelease(.02f);
    float input[8];

    MeasureTime<float>::run(
        overheadInOut, "multi lag", [&lpf, &input]() {
            float x = TestBuffers<float>::get();
            for (int i = 0; i < 8; ++i) {
                input[i] = x;
            }
            lpf.step(input);

            return lpf.get(3);
        },
        1);
}

static void testMultiLPFMod() {
    MultiLPF<8> lpf;

    lpf.setCutoff(.01f);
    float input[8];

    MeasureTime<float>::run(
        overheadInOut, "multi lpf mod", [&lpf, &input]() {
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
        },
        1);
}

static void testMultiLagMod() {
    MultiLag<8> lag;

    lag.setAttack(.01f);
    lag.setRelease(.02f);
    float input[8];

    MeasureTime<float>::run(
        overheadInOut, "multi lag mod", [&lag, &input]() {
            float x = TestBuffers<float>::get();
            float y = TestBuffers<float>::get();
            y = std::abs(y) + .1f;
            while (y > .49f) {
                y -= .48f;
            }
            assert(y > 0);
            assert(y < .5);

            lag.setAttack(y);
            lag.setRelease(y / 2);
            for (int i = 0; i < 8; ++i) {
                input[i] = x;
            }
            lag.step(input);

            return lag.get(3);
        },
        1);
}

static void testUniformLookup() {
    std::shared_ptr<LookupTableParams<float>> lookup = ObjectCache<float>::getSinLookup();
    MeasureTime<float>::run(
        overheadInOut, "uniform", [lookup]() {
            float x = TestBuffers<float>::get();
            return LookupTable<float>::lookup(*lookup, x, true);
        },
        1);
}
static void testNonUniform() {
    std::shared_ptr<NonUniformLookupTableParams<float>> lookup = makeLPFilterL_Lookup<float>();
    printf("non uniform lookup size = %d\n", lookup->size());

    MeasureTime<float>::run(
        overheadInOut, "non-uniform", [lookup]() {
            float x = TestBuffers<float>::get();
            return NonUniformLookupTable<float>::lookup(*lookup, x);
        },
        1);
    printf("Now abort");
    abort();
}

using Slewer = Slew4<TestComposite>;

static void testSlew4() {
    Slewer fs;

    fs.init();

    fs.inputs[Slewer::INPUT_AUDIO0].setVoltage(0, 0);

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(
        overheadInOut, "Slade", [&fs]() {
            fs.inputs[Slewer::INPUT_TRIGGER0].setVoltage(TestBuffers<float>::get(), 0);
            fs.step();
            return fs.outputs[Slewer::OUTPUT0].getVoltage(0);
        },
        1);
}

using DT = DrumTrigger<TestComposite>;
static void testDrumTrigger() {
    DT fs;
    fs.init();
    fs.inputs[DT::CV_INPUT].channels = 1;
    fs.inputs[DT::CV_INPUT].channels = 8;
    fs.outputs[DT::GATE0_OUTPUT].channels = 1;
    fs.outputs[DT::GATE0_OUTPUT].channels = 8;
    assert(overheadInOut >= 0);
    MeasureTime<float>::run(
        overheadInOut, "Polygate", [&fs]() {
            fs.inputs[DT::CV_INPUT].setVoltage(TestBuffers<float>::get(), 0);
            fs.step();
            return fs.outputs[DT::GATE0_OUTPUT].getVoltage(0);
        },
        1);
}

using Filter = Filt<TestComposite>;
static void testFilt() {
    Filter fs;
    fs.init();
    fs.inputs[Filter::L_AUDIO_INPUT].channels = 1;
    fs.outputs[Filter::L_AUDIO_OUTPUT].channels = 1;
    assert(overheadInOut >= 0);
    MeasureTime<float>::run(
        overheadInOut, "filt", [&fs]() {
            fs.inputs[Filter::L_AUDIO_INPUT].setVoltage(TestBuffers<float>::get(), 0);
            fs.step();
            return fs.outputs[Filter::L_AUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testFilt2() {
    Filter fs;
    fs.init();
    fs.inputs[Filter::L_AUDIO_INPUT].channels = 1;
    fs.outputs[Filter::L_AUDIO_OUTPUT].channels = 1;
    assert(overheadInOut >= 0);
    MeasureTime<float>::run(
        overheadInOut, "filt w/mod", [&fs]() {
            fs.inputs[Filter::L_AUDIO_INPUT].setVoltage(TestBuffers<float>::get(), 0);
            fs.params[Filter::FC_PARAM].value = TestBuffers<float>::get();
            fs.step();
            return fs.outputs[Filter::L_AUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

using Mixer8 = Mix8<TestComposite>;
static void testMix8() {
    Mixer8 fs;

    fs.init();

    fs.inputs[fs.AUDIO0_INPUT].setVoltage(0, 0);

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(
        overheadInOut, "mix8", [&fs]() {
            fs.inputs[Slewer::INPUT_TRIGGER0].setVoltage(TestBuffers<float>::get(), 0);
            fs.step();
            return fs.outputs[Slewer::OUTPUT0].getVoltage(0);
        },
        1);
}

using Mixer4 = Mix4<TestComposite>;
static void testMix4() {
    Mixer4 fs;
    fs.init();
    fs.inputs[fs.AUDIO0_INPUT].setVoltage(0, 0);

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(
        overheadInOut, "mix4", [&fs]() {
            fs.inputs[Slewer::INPUT_TRIGGER0].setVoltage(TestBuffers<float>::get(), 0);
            fs.step();
            return fs.outputs[Slewer::OUTPUT0].getVoltage(0);
        },
        1);
}

using MixerSt = MixStereo<TestComposite>;
static void testMixStereo() {
    MixerSt fs;
    fs.init();
    fs.inputs[fs.AUDIO0_INPUT].setVoltage(0, 0);

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(
        overheadInOut, "mix stereo", [&fs]() {
            fs.inputs[Slewer::INPUT_TRIGGER0].setVoltage(TestBuffers<float>::get(), 0);
            fs.step();
            return fs.outputs[Slewer::OUTPUT0].getVoltage(0);
        },
        1);
}

using MixerM = MixM<TestComposite>;
static void testMixM() {
    MixerM fs;

    fs.init();

    fs.inputs[fs.AUDIO0_INPUT].setVoltage(0, 0);

    assert(overheadInOut >= 0);
    MeasureTime<float>::run(
        overheadInOut, "mixM", [&fs]() {
            fs.inputs[Slewer::INPUT_TRIGGER0].setVoltage(TestBuffers<float>::get());
            fs.step();
            return fs.outputs[Slewer::OUTPUT0].getVoltage(0);
        },
        1);
}

static void testF2_Poly1() {
    using Comp = F2_Poly<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::AUDIO_INPUT].setVoltage(0, 0);
    comp.inputs[Comp::AUDIO_INPUT].channels = 1;

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "testF2:a mono 12/Lim", [&comp, args]() {
            comp.inputs[Comp::AUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testF2_Poly16() {
    using Comp = F2_Poly<TestComposite>;
    Comp comp;

    comp.init();

    //  comp.params[Comp::CV_UPDATE_FREQ].value = 0;
    comp.params[Comp::TOPOLOGY_PARAM].value = float(Comp::Topology::SERIES);
    comp.inputs[Comp::AUDIO_INPUT].channels = 16;
    for (int i = 0; i < 16; ++i) {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(0, i);
    }

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "testF2:c 24+lim 16 ch", [&comp, args]() {
            comp.inputs[Comp::AUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testF2_12nl() {
    using Comp = F2_Poly<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::AUDIO_INPUT].channels = 1;
    comp.params[Comp::TOPOLOGY_PARAM].value = float(Comp::Topology::SINGLE);
    comp.params[Comp::LIMITER_PARAM].value = 0;
    for (int i = 0; i < 16; ++i) {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(0, i);
    }

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "testF2:b (12/nl)", [&comp, args]() {
            comp.inputs[Comp::AUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testF2_24l() {
    using Comp = F2_Poly<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::AUDIO_INPUT].channels = 1;
    comp.inputs[Comp::FC_INPUT].channels = 1;
    comp.inputs[Comp::Q_INPUT].channels = 1;
    comp.inputs[Comp::R_INPUT].channels = 1;

    comp.params[Comp::TOPOLOGY_PARAM].value = float(Comp::Topology::SERIES);
    comp.params[Comp::LIMITER_PARAM].value = 1;

    comp.params[Comp::FC_TRIM_PARAM].value = 1;
    comp.params[Comp::Q_TRIM_PARAM].value = 1;
    comp.params[Comp::R_TRIM_PARAM].value = 1;

    for (int i = 0; i < 16; ++i) {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(0, i);
    }

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "testF2:d (24/lim mod q,r,fc mono)", [&comp, args]() {
            comp.inputs[Comp::AUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.inputs[Comp::Q_INPUT].setVoltage(TestBuffers<float>::get());
            comp.inputs[Comp::R_INPUT].setVoltage(TestBuffers<float>::get());
            comp.inputs[Comp::FC_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testF2_24l4() {
    using Comp = F2_Poly<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::AUDIO_INPUT].channels = 4;
    comp.inputs[Comp::FC_INPUT].channels = 4;
    comp.inputs[Comp::Q_INPUT].channels = 4;
    comp.inputs[Comp::R_INPUT].channels = 4;

    comp.params[Comp::TOPOLOGY_PARAM].value = float(Comp::Topology::SERIES);
    comp.params[Comp::LIMITER_PARAM].value = 1;

    comp.params[Comp::FC_TRIM_PARAM].value = 1;
    comp.params[Comp::Q_TRIM_PARAM].value = 1;
    comp.params[Comp::R_TRIM_PARAM].value = 1;

    for (int i = 0; i < 16; ++i) {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(0, i);
    }

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "testF2:e (24/lim mod q,r,fc) 4ch", [&comp, args]() {
            comp.inputs[Comp::AUDIO_INPUT].setVoltageSimd(TestBuffers<float>::get4(), 0);
            comp.inputs[Comp::Q_INPUT].setVoltageSimd(TestBuffers<float>::get4(), 0);
            comp.inputs[Comp::R_INPUT].setVoltageSimd(TestBuffers<float>::get4(), 0);
            comp.inputs[Comp::FC_INPUT].setVoltageSimd(TestBuffers<float>::get4(), 0);
            comp.process(args);
            return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testF2_24l_4() {
    using Comp = F2_Poly<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::AUDIO_INPUT].channels = 4;
    comp.params[Comp::TOPOLOGY_PARAM].value = float(Comp::Topology::SERIES);
    comp.params[Comp::LIMITER_PARAM].value = 1;
    //  comp.params[Comp::CV_UPDATE_FREQ].value = 1;
    for (int i = 0; i < 16; ++i) {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(0, i);
    }

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "testF2:f (24/lim) 4ch", [&comp, args]() {
            comp.inputs[Comp::AUDIO_INPUT].setVoltageSimd(TestBuffers<float>::get4(), 0);
            // comp.inputs[Comp::FC_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testF2_g() {
    using Comp = F2_Poly<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::AUDIO_INPUT].channels = 1;
    comp.params[Comp::TOPOLOGY_PARAM].value = float(Comp::Topology::SERIES);
    comp.params[Comp::LIMITER_PARAM].value = 1;
    //  comp.params[Comp::CV_UPDATE_FREQ].value = 1;
    for (int i = 0; i < 16; ++i) {
        comp.inputs[Comp::AUDIO_INPUT].setVoltage(0, i);
    }

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "testF2:g (24/lim) 1ch", [&comp, args]() {
            comp.inputs[Comp::AUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testCompLim1() {
    using Comp = Compressor<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::LAUDIO_INPUT].channels = 1;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 0;  // limiter
    comp.params[Comp::NOTBYPASS_PARAM].value = 1;

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "Comp/Lim 1 channel", [&comp, args]() {
            comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testCompLim16() {
    using Comp = Compressor<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::LAUDIO_INPUT].channels = 16;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 0;  // limiter
    comp.params[Comp::NOTBYPASS_PARAM].value = 1;

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "Comp/Lim 16 channel", [&comp, args]() {
            comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testCompKnee() {
    using Comp = Compressor<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::LAUDIO_INPUT].channels = 1;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 3;  // 4:1 hard knee
    comp.params[Comp::NOTBYPASS_PARAM].value = 1;

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "Comp / Lim 1 channel 4:1 soft", [&comp, args]() {
            comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testCompKnee16() {
    using Comp = Compressor<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::LAUDIO_INPUT].channels = 16;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 3;  // 4:1 sort knee
    comp.params[Comp::NOTBYPASS_PARAM].value = 1;
    printf("setting ratio to 1\n");

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "Comp / Lim 16 channel 4:1 soft", [&comp, args]() {
            comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testComp2Knee16() {
    using Comp = Compressor2<TestComposite>;
    Comp comp;

    comp.init();
    initComposite(comp);

    comp.inputs[Comp::LAUDIO_INPUT].channels = 16;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 3;  // 4:1 sort knee
    comp.params[Comp::NOTBYPASS_PARAM].value = 1;
    comp.params[Comp::STEREO_PARAM].value = 0;

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "Comp2 16 channel 4:1 soft", [&comp, args]() {
            comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testComp2Knee16Linked() {
    using Comp = Compressor2<TestComposite>;
    Comp comp;

    comp.init();
    initComposite(comp);

    comp.inputs[Comp::LAUDIO_INPUT].channels = 16;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 3;  // 4:1 sort knee
    comp.params[Comp::NOTBYPASS_PARAM].value = 1;
    comp.params[Comp::STEREO_PARAM].value = 2;

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "Comp2 16 channel 4:1 soft linkes-s", [&comp, args]() {
            comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        },
        1);
}
//using Comp2 = Compressor2<TestComposite>;
static void run(Compressor2<TestComposite>& comp, int times) {
    TestComposite::ProcessArgs args;
    for (int i = 0; i < times; ++i) {
        comp.process(args);
    }
}

static void testComp2Knee16Bypassed() {
    using Comp = Compressor2<TestComposite>;
    Comp comp;

    comp.init();
    initComposite(comp);

    comp.inputs[Comp::LAUDIO_INPUT].channels = 16;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 3;  // 4:1 sort knee
    comp.params[Comp::NOTBYPASS_PARAM].value = 0;
    SQINFO("wrote 0 in test");
    comp.params[Comp::STEREO_PARAM].value = 0;

    assert(int(std::round(comp.params[Comp::CHANNEL_PARAM].value)) == 1);
    assert(bool(std::round(comp.params[Comp::NOTBYPASS_PARAM].value)) == 0);

    run(comp, 40);

    // What's happening is that we see a "new" current channel and that makes us re-init the params.
    // Is this a bug? Why are we even seeing a change here?
    //assert(bool(std::round(comp.params[Comp::NOTBYPASS_PARAM].value)) == 0);
    comp.params[Comp::NOTBYPASS_PARAM].value = 0;
    run(comp, 40);

    comp.ui_setAllChannelsToCurrent();
    assert(bool(std::round(comp.params[Comp::NOTBYPASS_PARAM].value)) == 0);
    run(comp, 40);
    run(comp, 1);

    assert( bool(std::round(comp.params[Comp::NOTBYPASS_PARAM].value)) == 0);

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "Comp2 16 channel 4:1 soft bypassed", [&comp, args]() {
            comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testCompKnee16Hard() {
    using Comp = Compressor<TestComposite>;
    Comp comp;

    comp.init();

    comp.inputs[Comp::LAUDIO_INPUT].channels = 16;
    comp.inputs[Comp::LAUDIO_INPUT].setVoltage(0, 0);
    comp.params[Comp::RATIO_PARAM].value = 2;  // 4:1 hard knee
    comp.params[Comp::NOTBYPASS_PARAM].value = 1;

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;

    MeasureTime<float>::run(
        overheadInOut, "Comp / Lim 16 channel 4:1 hard", [&comp, args]() {
            comp.inputs[Comp::LAUDIO_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::LAUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

void perfTest2() {
    assert(overheadInOut > 0);
    assert(overheadOutOnly > 0);

    testComp2Knee16Bypassed();
    testComp2Knee16();
    testComp2Knee16Linked();
 

    testCompLim1();
    testCompLim16();
    testCompKnee();
    testCompKnee16();
    testCompKnee16Hard();

    testF2_24l();
    testF2_Poly1();
    testF2_Poly16();
    testF2_12nl();
    testF2_24l();
    testF2_24l4();
    testF2_24l_4();
    testF2_g();

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