
#include <assert.h>

#include "Analyzer.h"
#include "asserts.h"
#include "CHB.h"
#include "EvenVCO.h"
#include "FunVCO.h"
#include "SawOscillator.h"
#include "TestComposite.h"


using EVCO = EvenVCO <TestComposite>;
//using FUN = VoltageControlledOscillator<16, 16>;
using CH = CHB<TestComposite>;

float desiredPitchEv(const EVCO& vco)
{
    // This is just the original code as reference
    float pitch = 1.0f + roundf(vco.params[(int) EVCO::OCTAVE_PARAM].value) + vco.params[(int) EVCO::TUNE_PARAM].value / 12.0f;
    pitch += vco.inputs[(int) EVCO::PITCH1_INPUT].value + vco.inputs[(int) EVCO::PITCH2_INPUT].value;
    pitch += vco.inputs[(int) EVCO::FM_INPUT].value / 4.0f;

    float freq = 261.626f * powf(2.0f, pitch);
   // printf("theirs: pitch = %f exp = %f\n", pitch, freq);
    return freq;
}

float desiredPitchCh(const CH& vco)
{

    float pitch = 1.0f + roundf(vco.params[(int) CH::PARAM_OCTAVE].value) + vco.params[(int) CH::PARAM_TUNE].value / 12.0f;
    pitch += vco.inputs[(int) CH::CV_INPUT].value;
    pitch += vco.inputs[(int) CH::PITCH_MOD_INPUT].value / 4.0f;

    // TODO: atenuverter on FM

    float freq = 261.626f * powf(2.0f, pitch);
    // printf("theirs: pitch = %f exp = %f\n", pitch, freq);
    return freq;
}

static void testxEv(float octave, float tune = 0, float pitch1 = 0, float pitch2 = 0, float fm = 0)
{
    EVCO vco;

    vco.params[(int) EVCO::OCTAVE_PARAM].value = octave;
    vco.params[(int) EVCO::TUNE_PARAM].value = tune;
    vco.inputs[(int) EVCO::PITCH1_INPUT].value = pitch1;
    vco.inputs[(int) EVCO::PITCH2_INPUT].value = pitch2;
    vco.inputs[(int) EVCO::FM_INPUT].value = fm;

    vco.outputs[(int) EVCO::SAW_OUTPUT].active = true;
    vco.outputs[(int) EVCO::EVEN_OUTPUT].active = false;
    vco.outputs[(int) EVCO::TRI_OUTPUT].active = false;
    vco.outputs[(int) EVCO::SQUARE_OUTPUT].active = false;
    vco.outputs[(int) EVCO::SINE_OUTPUT].active = false;

    vco.step();
    const float desired = desiredPitchEv(vco);

   // printf("test, oct=%f, freq=%.2f desired=%.2f\n", octave, vco._freq, desired);
    if (desired > 20000) {
        // lookup table doesn't go past 20k. that's fine
        assertGE(vco._freq, 20000 - 1);
    } else {
        assertClose(vco._freq, desired, 1.5);     // todo: make better tolerance
    }
}

static void testxCh(float octave, float tune = 0, float pitch1 = 0, float pitch2 = 0, float fm = 0)
{
    CH vco;

    assert(pitch2 == 0);     // ch doesn't have one

    vco.params[(int) CH::PARAM_OCTAVE].value = octave;
    vco.params[(int) CH::PARAM_TUNE].value = tune;
    vco.inputs[(int) CH::CV_INPUT].value = pitch1;
  //  vco.inputs[(int) CH::PITCH2_INPUT].value = pitch2;
    vco.inputs[(int) CH::PITCH_MOD_INPUT].value = fm;

#if 0
    vco.outputs[(int) CH::SAW_OUTPUT].active = true;
    vco.outputs[(int) CH::EVEN_OUTPUT].active = false;
    vco.outputs[(int) CH::TRI_OUTPUT].active = false;
    vco.outputs[(int) CH::SQUARE_OUTPUT].active = false;
    vco.outputs[(int) CH::SINE_OUTPUT].active = false;
#endif

    vco.step();
    const float desired = desiredPitchCh(vco);

    // printf("test, oct=%f, freq=%.2f desired=%.2f\n", octave, vco._freq, desired);
    if (desired > 20000) {
        // lookup table doesn't go past 20k. that's fine
        assertGE(vco._freq, 20000 - 1);
    } else {
        assertClose(vco._freq, desired, 1.5);     // todo: make better tolerance
    }
}


static void testInitEv()
{
    EVCO vco;

    vco.step();
    const float desired = desiredPitchEv(vco);
    assertClose(vco._freq, desired, 1);         // todo: tighten up
}

static void testInitCh()
{
    CH vco;

    vco.step();
    const float desired = desiredPitchCh(vco);
    assertClose(vco._freq, desired, 1);         // todo: tighten up
}

static void testOctavesEv()
{
    EVCO vco;
    for (int octave = -5; octave <= 4; ++octave) {
        testxEv(float(octave));
    }
}

static void testOctavesCh()
{
    CH vco;
    for (int octave = -5; octave <= 4; ++octave) {
        testxCh(float(octave));
    }
}
// test that we go up to 20k
static void testMaxFreqEv()
{
    testxEv(4, 7, 0, 0);
    testxEv(4, 7, 1, 0);
    testxEv(4, 7, 0, 1);

}

static void testMaxFreqCh()
{
    testxCh(4, 7, 0, 0);
    testxCh(4, 7, 1, 0);
   // testxCh(4, 7, 0, 1);
}

static void testMinFreqEv()
{
    testxEv(-5, -7, 0, 0);
    testxEv(-5, -7, -2, 0);
}
static void testMinFreqCh()
{
    testxCh(-5, -7, 0, 0);
    testxCh(-5, -7, -2, 0);
}
void testVCO()
{
    testInitEv();
    testInitCh();
    testOctavesEv();
    testOctavesCh();
    testMaxFreqEv();
    testMaxFreqCh();
    testMinFreqEv();
    testMinFreqCh();
}