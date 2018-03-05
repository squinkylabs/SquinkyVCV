#include <assert.h>
#include <iostream>

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
       // assert(anim.filterFrequency[i] == anim.nominalFilterCenter[i]);
        assert(false);
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
#if 0
    for (int i = 0; i < 4; ++i) {
        assert(anim.filterFrequency[i] == anim.nominalFilterCenter[i]);
    }

    anim.params[anim.FILTER_FC_PARAM].value = 1;
    anim.step();
    assert(false);

    for (int i = 0; i < 4; ++i) {
        if (i == 3)
            assert(anim.filterFrequency[i] == anim.nominalFilterCenter[i]);
        else
            assert(anim.filterFrequency[i] > anim.nominalFilterCenter[i]);
    }
    anim.params[anim.FILTER_FC_PARAM].value = -1;
    anim.step();
    for (int i = 0; i < 4; ++i) {
        if (i == 3)
            assert(anim.filterFrequency[i] == anim.nominalFilterCenter[i]);
        else
            assert(anim.filterFrequency[i] < anim.nominalFilterCenter[i]);
    }
#endif
}

static void dump(const char * msg, const Animator& anim)
{
    std::cout << "dumping " << msg << "\nfiltFreq"
        << " " << std::pow(2, anim.filterFrequencyLog[0])
        << " " << std::pow(2, anim.filterFrequencyLog[1])
        << " " << std::pow(2, anim.filterFrequencyLog[2])
        << " " << std::pow(2, anim.filterFrequencyLog[3])
        << std::endl;
}
static void x()
{
    Animator anim;
    anim.setSampleRate(44100);
    anim.init();
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.step();

    dump("init", anim);
    
    // TODO: assert here
    anim.params[anim.FILTER_FC_PARAM].value = 5;
    anim.step();
    dump("fc 5", anim);
   
    anim.params[anim.FILTER_FC_PARAM].value = -5;
    anim.step();
    dump("fc -5", anim);

    std::cout << "\nabout to modulate up. maxLFO, def depth\n";
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.params[anim.FILTER_MOD_DEPTH_PARAM].value = 0;
    anim.jamModForTest = true;
    anim.modValueForTest = 5;
    anim.step();
    dump("max up def", anim);

    std::cout << "\nabout to modulate up. minLFO, def depth\n";
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.params[anim.FILTER_MOD_DEPTH_PARAM].value = 0;
    anim.jamModForTest = true;
    anim.modValueForTest = -5;
    anim.step();
    dump("max down def", anim);

    std::cout << "\nabout to modulate up. maxLFO, max depth\n";
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.params[anim.FILTER_MOD_DEPTH_PARAM].value = 5;
    anim.jamModForTest = true;
    anim.modValueForTest = 5;
    anim.step();
    dump(" modulate up. maxLFO, max depthf", anim);


    std::cout << "\nabout to modulate down. minLFO, max depth\n";
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.params[anim.FILTER_MOD_DEPTH_PARAM].value = 5;
    anim.jamModForTest = true;
    anim.modValueForTest = -5;
    anim.step();
    dump(" modulate up. maxLFO, max depthf", anim);


#if 0
    // TODO: would be nice to be able to inject an LFO voltage
    anim.params[anim.FILTER_FC_PARAM].value = 0;  
    anim.params[anim.FILTER_MOD_DEPTH_PARAM].value = 5;
    for (int i = 0; i < 40000; ++i) {
        anim.step();
    }
    dump("fc 0 depth 1", anim);

    std::cout << "about to to depth -\n";
    // TODO: would be nice to be able to inject an LFO voltage
    anim.params[anim.FILTER_FC_PARAM].value = 0;
    anim.params[anim.FILTER_MOD_DEPTH_PARAM].value = -5;
    for (int i = 0; i < 4000; ++i) {
        anim.step();
    }
    dump("fc 0 depth -5", anim);
#endif
  
   
 
}


void testVocalAnimator()
{

    test0();
    test1();
    test2();
    test3();
    x();

}