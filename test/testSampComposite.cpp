

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

   // assert(channelToTest == 0);

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

    SQWARN("-------------------------------------------");
    SQWARN("        put test back once we settle on trim");
    SQWARN("-------------------------------------------");

    // testMod(channel, .5, 1, 1.11612f);
}

static void testSampComposite0() {
    for (int i = 0; i < 16; ++i) {
        testSampComposite0(i);
    }
}
    /*
        zeroMix(sub);
    sub.inputs[Comp::VOCT_INPUT].channels = 8;  // this test requires poly
    sub.params[Comp::VCO1_LEVEL_PARAM].value = 100;
    sub.inputs[Comp::MAIN1_LEVEL_INPUT].channels = 1;
    sub.inputs[Comp::MAIN1_LEVEL_INPUT].setVoltage(10, 0);      // 10 v on channel 0
    sub.inputs[Comp::MAIN1_LEVEL_INPUT].setChannels(8);
    assert(sub.inputs[Comp::MAIN1_LEVEL_INPUT].isConnected());
    */




void testSampComposite() {
    testSampComposite0();

}