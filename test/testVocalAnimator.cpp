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

void testVocalAnimator()
{
    test0();
    test1();
}