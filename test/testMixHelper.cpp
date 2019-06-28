
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
}

void testMixHelper()
{
    test0();
}