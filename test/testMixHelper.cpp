
#include "asserts.h"
#include "MixHelper.h"
#include "TestComposite.h"

class MockMixComposite : public TestComposite
{
public:
    static const int numChannels = 4;


    enum ParamIds
    {
        MUTE0_PARAM,
        MUTE1_PARAM,
        MUTE2_PARAM,
        MUTE3_PARAM,
        MUTE0_STATE_PARAM,
        MUTE1_STATE_PARAM,
        MUTE2_STATE_PARAM,
        MUTE3_STATE_PARAM

    };

    enum InputIds
    {
        MUTE0_INPUT,
        MUTE1_INPUT,
        MUTE2_INPUT,
        MUTE3_INPUT
    };
};




// just make it all compile
static void test0()
{
    MockMixComposite comp;
    MixHelper< MockMixComposite> helper;
    helper.procMixInputs(&comp);


    for (int i = 0; i < 4; ++i) {
        assertEQ(comp.params[MockMixComposite::MUTE0_PARAM + i].value, 0);
        assertEQ(comp.params[MockMixComposite::MUTE0_STATE_PARAM + i].value, 0);
    }
}


// test the calling the function does nothing when nothing to do
static void testNothing()
{
    MockMixComposite comp;
    MixHelper< MockMixComposite> helper;
    for (int i = 0; i < 4; ++i) {
        helper.procMixInputs(&comp);
    }

    for (int i = 0; i < 4; ++i) {
        assertEQ(comp.params[MockMixComposite::MUTE0_PARAM + i].value, 0);
        assertEQ(comp.params[MockMixComposite::MUTE0_STATE_PARAM + i].value, 0);
    }

}



static void testParamToggle(int channel)
{
    MockMixComposite comp;
    MixHelper< MockMixComposite> helper;
    
    // let the inputs see zero
    helper.procMixInputs(&comp);

    // send it a mute toggle
    comp.params[MockMixComposite::MUTE0_PARAM + channel].value = 1;
    helper.procMixInputs(&comp);

    // check the results
    for (int i = 0; i < 4; ++i) {
        const float expectedMuteState = (i == channel) ? 1.f : 0.f;
        assertEQ(comp.params[MockMixComposite::MUTE0_STATE_PARAM + i].value, expectedMuteState);
    }

    // mute back down to zero
    comp.params[MockMixComposite::MUTE0_PARAM + channel].value = 0;
    helper.procMixInputs(&comp);
    // check the results - should be unchanged
    for (int i = 0; i < 4; ++i) {
        const float expectedMuteState = (i == channel) ? 1.f : 0.f;
        assertEQ(comp.params[MockMixComposite::MUTE0_STATE_PARAM + i].value, expectedMuteState);
    }

    // mute up again should toggle
    comp.params[MockMixComposite::MUTE0_PARAM + channel].value = 1;
    helper.procMixInputs(&comp);
    // check the results - should be unchanged
    for (int i = 0; i < 4; ++i) {
        const float expectedMuteState = 0;
        assertEQ(comp.params[MockMixComposite::MUTE0_STATE_PARAM + i].value, expectedMuteState);
    }

}

static void testParamToggle()
{
    for (int i = 0; i < 4; ++i) {
        testParamToggle(i);
    }
}

void testMixHelper()
{
    test0();
    testNothing();
    testParamToggle();
}