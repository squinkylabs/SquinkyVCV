#include <assert.h>

#include "VocalAnimator.h"
#include "TestComposite.h"

using Animator = VocalAnimator<TestComposite>;

/**
 * Verify no output with no input.
 */
static void test0()
{
    Animator anim;
    anim.setSampleRate(44100);
    anim.init();

    anim.outputs[Animator::AUDIO_OUTPUT].value = 0;
    anim.step();                // prime it

                                // with no input, should have no output
    for (int i = 0; i < 50; ++i) {
        anim.step();
        assert(anim.outputs[Animator::AUDIO_OUTPUT].value == 0);
    }
}

/**
 * Verify output with input.
 */
static void test1()
{
    Animator anim;
    anim.setSampleRate(44100);
    anim.init();

    anim.outputs[Animator::AUDIO_OUTPUT].value = 0;
    anim.inputs[Animator::AUDIO_INPUT].value = 1;
    anim.step();                // prime it
                                // with  input, should have  output
    for (int i = 0; i < 50; ++i) {
        anim.step();
        assert(anim.outputs[Animator::AUDIO_OUTPUT].value != 0);
    }
}

/**
 * Verify filter settings with no mod.
 */
static void test2()
{
    Animator anim;
    anim.setSampleRate(44100);
    anim.init();
    for (int i = 0; i < 4; ++i) {
        assert(anim.filterFrequencies[i] == anim.nominalFilterCenters[i]);
    }
}

/**
* Verify filter settings respond to Fc.
*/
static void test3()
{
    Animator anim;
    anim.setSampleRate(44100);
    anim.init();
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.step();
    for (int i = 0; i < 4; ++i) {
        assert(anim.filterFrequencies[i] == anim.nominalFilterCenters[i]);
    }

    anim.params[anim.FILTER_FC_PARAM].value = 1;
    anim.step();
    for (int i = 0; i < 4; ++i) {
        assert(anim.filterFrequencies[i] > anim.nominalFilterCenters[i]);
    }
    anim.params[anim.FILTER_FC_PARAM].value = -1;
    anim.step();
    for (int i = 0; i < 4; ++i) {
        assert(anim.filterFrequencies[i] < anim.nominalFilterCenters[i]);
    }
}
void testVocalAnimator()
{
    test0();
    test1();
    test2();
}