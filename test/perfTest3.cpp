
#include "MeasureTime.h"
#include "Samp.h"

extern double overheadOutOnly;
extern double overheadInOut;

static void testSamp1() {
    using Comp = Samp<TestComposite>;
    Comp comp;

    comp.init();

    //  comp.inputs[Comp::AUDIO_INPUT].setVoltage(0, 0);
    //   comp.inputs[Comp::AUDIO_INPUT].channels = 1;

    Comp::ProcessArgs args;
    args.sampleTime = 1.f / 44100.f;
    args.sampleRate = 44100;
    int iter = 0;

    // need to make this real. need samples to play, and stuff.

    MeasureTime<float>::run(
        overheadInOut, "testSamp basic", [&comp, args, &iter]() {
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
}
