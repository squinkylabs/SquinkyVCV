#pragma once

#include <functional>
#include <tuple>

#include "TestComposite.h"
#include "asserts.h"

// TODO: move these to utils
template <class T>
inline void initComposite(T& comp)
{
    comp.init(); 
    auto icomp = comp.getDescription();
    for (int i = 0; i < icomp->getNumParams(); ++i) {
        auto param = icomp->getParam(i);
        comp.params[i].value = param.def;
    }
}

/**
 * returns mix:min:average
 */
inline std::tuple<float, float, float> getSignalStats(int iterations, std::function<float(void)> lambda)
{
    float positive = -100;
    float negative = 100; 
    float sum = 0; 
    for (int i=0; i < iterations; ++i) {  
        float x = lambda();
        sum += x;
        positive = std::max(positive, x); 
        negative = std::min(negative, x);  
    } 
    return std::make_tuple(negative, positive, sum / iterations);
}

/**
 * will instantiate a T,
 * hook it up for numChannels,
 * put something in each input, test that it comes our each output.
 */
template <class T>
void testPolyChannels(int  inputPort, int outputPort, int numChannels)
{
    T comp;
    initComposite(comp);
    comp.inputs[inputPort].channels = numChannels;
    comp.outputs[outputPort].channels = 1;          // this will set it as patched so comp can set it right

    TestComposite::ProcessArgs args =  {44100, 1/44100}; 

    comp.process(args);
    // const int outputChannels = comp.outputs[outputPort].channels;

    assertEQ(int(comp.outputs[outputPort].channels), numChannels);
    for (int i = 0; i < numChannels; ++i) {

    #if 0
       // printf("i = %d\n", i);
        // should start out at zero output
        if (i < 4) {
            for (int j=0; j < 4; ++j) {
                printf("channel %d, looking at %d output = %f\n", i, j, comp.outputs[outputPort].getVoltage(i));  fflush(stdout);
            }
        }
    #endif
        assertEQ(comp.outputs[outputPort].getVoltage(i), 0);
        comp.inputs[inputPort].setVoltage(10, i);
       // for (int j=0; j<100; ++j) {
            comp.process(args);
        assertGT(comp.outputs[outputPort].getVoltage(i), 0);
    }
}


/**
 * This test routine will:
 *      instantiate a T (usually a composite)
 *      initialize it.
 *      call @param setup
 *      process long enough for it to take effect
 *      call @param validate, which will usually assert on state
 */
template <class T>
inline void testArbitrary( std::function<void(T&)> setup, std::function<void(T&)> validate)
{
    T comp;
    initComposite(comp);

    setup(comp);
    TestComposite::ProcessArgs args =  {44100, 1/44100}; 
    for (int i = 0; i < 40; ++i) {
        comp.process(args);
    }
    validate(comp);
}