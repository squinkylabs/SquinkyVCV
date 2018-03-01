#include <assert.h>
#include <vector>

#include "VocalAnimator.h"
#include "FrequencyShifter.h"
#include "TestComposite.h"

using Shifter = FrequencyShifter<TestComposite>;
using Animator = VocalAnimator<TestComposite>;

// just test the can compile, etc..
static void test0()
{
    Shifter fs;
    fs.setSampleRate(44100);
    fs.init();
    fs.step();
}

// test for signal
static void test1()
{
    Shifter fs;

    fs.setSampleRate(44100);
    fs.init();

    fs.inputs[Shifter::AUDIO_INPUT].value = 0;
    fs.outputs[Shifter::SIN_OUTPUT].value = 0;

    // with no input, should have no output
    for (int i = 0; i < 50; ++i) {
        fs.step();
        assert(fs.outputs[Shifter::SIN_OUTPUT].value == 0);
    }

    fs.inputs[Shifter::AUDIO_INPUT].value = 1;
    // this should produce output
    for (int i = 0; i < 50; ++i) {
        fs.step(); 
        assert(!AudioMath::closeTo(fs.outputs[Shifter::SIN_OUTPUT].value, 0, .00001));
        assert(!AudioMath::closeTo(fs.outputs[Shifter::COS_OUTPUT].value, 0, .00001));
    }
}

static void test2()
{
    Animator anim;
    anim.setSampleRate(44100);
    anim.init();

    anim.outputs[Animator::MAIN_OUTPUT].value = 0;
    anim.step();                // prime it

    // with no input, should have output (for now)
    for (int i = 0; i < 50; ++i) {
        anim.step();
        assert(anim.outputs[Animator::MAIN_OUTPUT].value != 0);
    }
}


void testFrequencyShifter()
{
    test0();
    test1();
    test2();
}