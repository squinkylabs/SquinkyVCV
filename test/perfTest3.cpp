

#include "MeasureTime.h"
#include "Samp.h"

extern double overheadOutOnly;
extern double overheadInOut;

static void testSamp1() {
    using Comp = Samp<TestComposite>;
    Comp comp;

    comp.init();
    comp._setupPerfTest();
    comp.inputs[Comp::PITCH_INPUT].channels = 1;

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;
  //  int iter = 0;
    MeasureTime<float>::run(
        overheadInOut, "sampler at rest", [&comp, args]() {
            // comp.inputs[Comp::A_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

static void testSamp2() {
    using Comp = Samp<TestComposite>;
    Comp comp;

    comp.init();
    comp._setupPerfTest();
    comp.inputs[Comp::PITCH_INPUT].channels = 1;
    comp.inputs[Comp::GATE_INPUT].channels = 1;
    comp.inputs[Comp::GATE_INPUT].setVoltage(10, 0);    // 10v gate in input 0

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;
    MeasureTime<float>::run(
        overheadInOut, "sampler play one note", [&comp, args]() {
            // comp.inputs[Comp::A_INPUT].setVoltage(TestBuffers<float>::get());
            comp.process(args);
            return comp.outputs[Comp::AUDIO_OUTPUT].getVoltage(0);
        },
        1);
}

void perfTest3() {
    assert(overheadInOut > 0);
    assert(overheadOutOnly > 0);
    testSamp1();
    testSamp2();
}
