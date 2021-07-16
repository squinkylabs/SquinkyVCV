

#include "Samp.h"
#include "asserts.h"
#include "tutil.h"
using Comp = Samp<TestComposite>;

static void process(Comp& comp, int times) {
    Comp::ProcessArgs args;
    for (int i = 0; i < times; ++i) {
        comp.process(args);
    }
}

static void testMod(int channelToTest, float trimValue, float cvValue, float expectedTranspose) {
    Comp comp;
    initComposite(comp);
    comp.suppressErrors();

    comp.inputs[Comp::PITCH_INPUT].channels = 16;
    
    comp.params[Comp::PITCH_TRIM_PARAM].value = trimValue;        // turn pitch vc trim up
    comp.inputs[Comp::GATE_INPUT].setVoltage(5, channelToTest);  // set gate high on channel 0
    comp.inputs[Comp::FM_INPUT].setVoltage(cvValue, channelToTest);  // 1V fm
    process(comp, 32);

    float x = comp._getTranspose(channelToTest);
    assertClose(x, expectedTranspose, .0001f);
}

static void testSampComposite0(int channel) {
    testMod(channel, 0, 0, 1);
    testMod(channel, 1, 1, 2);
    testMod(channel, -1, 1, .5);

    testMod(channel, .5, 1, 1.02798f);      // I just put in known good, but that's ok with me.
}

static void testSampComposite0() {
    for (int i = 0; i < 16; ++i) {
        testSampComposite0(i);
    }
}

static void testSampCompositeError() {
    Comp::ProcessArgs args;
    Comp comp;
    initComposite(comp);
    comp.setNewSamples_UI("abcdef");
    for (bool done=false; !done; ) {
        if (comp.isNewInstrument_UI()) { done = true; }
        comp.process(args);
    }
    auto inst = comp.getInstrumentInfo_UI();
    assertEQ(inst->errorMessage, "can't open abcdef");
}

static void testSchemaUpdate(float oldPitch, float newOctave, float newPitch) {
    Comp comp;
    initComposite(comp);
    comp.suppressErrors();

    comp.params[Comp::PITCH_PARAM].value = oldPitch;
    comp.params[Comp::SCHEMA_PARAM].value = 0;
    Comp::ProcessArgs args;

    comp.process(args);
   // SQINFO("old %f gives %f,%f", oldPitch, comp.params[Comp::OCTAVE_PARAM].value, comp.params[Comp::PITCH_PARAM].value);
    
    const float octave = comp.params[Comp::OCTAVE_PARAM].value;
    const float fraction = comp.params[Comp::PITCH_PARAM].value;

    assertClose((octave + fraction), (oldPitch + 4), .0001);
    assertLE(fraction, .5);
    assertGE(fraction, -.5);

    assertEQ(comp.params[Comp::SCHEMA_PARAM].value, 1)  
    assertEQ(octave, newOctave);

    assertClose(fraction, newPitch, .0001);
}

static void testSchemaUpdate() {
    testSchemaUpdate(0, 4, 0);      /// old pitch 0 was c4
    testSchemaUpdate(.1f, 4, .1f);
    testSchemaUpdate(.4f, 4, .4f);
    testSchemaUpdate(.6f, 5, -.4f);
    testSchemaUpdate(.9f, 5, -.1f);
    testSchemaUpdate(1.1f, 5, .1f);

    testSchemaUpdate(-.1f, 4, -.1f); 
    testSchemaUpdate(-.4f, 4, -.4f);
    testSchemaUpdate(-.6f, 3, .4f);
    testSchemaUpdate(-.9f, 3, .1f);
}

void testSampComposite() {
    testSchemaUpdate();
    testSampComposite0();
    testSampCompositeError();
 
}