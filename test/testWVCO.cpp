
#include "tutil.h"
#include "TestComposite.h"
#include "WVCO.h"
#include "ADSR16.h"
#include "asserts.h"


using Comp = WVCO<TestComposite>;

static void testTriFormula()
{
    float_4 a;
    float_4 b;
    TriFormula::getLeftA(a, float_4(.5));
    simd_assertEQ(a, float_4(2));

    TriFormula::getLeftA(a, .1);
    simd_assertEQ(a, float_4(10));

    TriFormula::getRightAandB(a, b, .5);
    simd_assertEQ(a, float_4(-2));
    simd_assertEQ(b, float_4(2));
}


static void testTriFormula2()
{
    for (float _k = .1; _k < 1; _k += .09f) {
        float_4 k = _k;
      
        float_4 a, b;
        TriFormula::getLeftA(a, k);
  
        simd_assertEQ((k * a), float_4(1));
   
        TriFormula::getRightAandB(a, b, k);
        simd_assertEQ((k * a + b), float_4(1));
        simd_assertEQ((1.f *a + b), float_4(0));

    }
}


static void testADSR0()
{
    ADSR16 adsr;
    adsr.setA(0);
    adsr.setD(0);
    adsr.setS(0);
    adsr.setR(0);
    float_4 gates[4];
    const float sampleTime = 1.f / 44100.f;
    adsr.step(gates, sampleTime); 
}

static void testADSR1()
{ 
    const float sampleTime = 1.f / 44100.f;
    // very fast envelope
    ADSR16 adsr;
    adsr.setA(0);
    adsr.setD(0);
    adsr.setS(1); 
    adsr.setR(0); 
    adsr.setNumChannels(4);

    float_4 gates[4] = {0};

    for (int i=0; i<5; ++i) {
        adsr.step(gates, sampleTime);
    }

    gates[0] = float_4::mask(); 
    for (int i=0; i<5; ++i) {  
        adsr.step(gates, sampleTime); 
    }
   // should have attacked a bit 
    float_4 out = adsr.env[0]; 
    simd_assertGT(out, float_4(.1)); 
}

static void testADSR2()
{ 
    const float sampleTime = 1.f / 44100.f;
    // very fast envelope
    ADSR16 adsr;
    adsr.setA(0);
    adsr.setD(0);
    adsr.setS(1); 
    adsr.setR(0); 
    adsr.setNumChannels(4);

    float_4 gates[4] = {0};

    for (int i=0; i<5; ++i) {
        adsr.step(gates, sampleTime);
    }

    gates[0] = float_4::mask(); 

    for (int i=0; i<500; ++i) {  
        adsr.step(gates, sampleTime); 
    }
    
   // should reach steady sustain max
    float_4 out = adsr.env[0]; 
    simd_assertClose(out, float_4(1), .01);
}

static void testADSR3()
{
    ADSR16 adsr;
    for (int i=0; i<4; ++i) {
        simd_assertEQ(adsr.env[i], float_4(0));
    }
}

static void testADSRSingle(int num)
{
    assertGE(num, 0);
    assertLE(num, 15);

    const float sampleTime = 1.f / 44100.f;
    // very fast envelope
    ADSR16 adsr;
    adsr.setA(0);
    adsr.setD(0);
    adsr.setS(1); 
    adsr.setR(0); 
    adsr.setNumChannels(16);

    float_4 gates[4] = {0};

    for (int i=0; i<5; ++i) {
        adsr.step(gates, sampleTime);
    }

    const int bank = num / 4;
    const int index = num - 4 * bank;
    const float singleMask = float_4::mask()[0];
    gates[bank][index] = singleMask;

    for (int i=0; i<500; ++i) {  
        adsr.step(gates, sampleTime); 
    }
    
    for (int i = 0; i<16; ++i) {
        float expected = (i == num) ? 1.f : 0.f;

        const int bank = i / 4;
        const int index = i - 4 * bank;
        float actual = adsr.env[bank][index];

        assertClose(actual, expected, .01);
    }
}

static void testADSRSingle()
{
    for (int i=0; i<16; ++i) {
        testADSRSingle(i);
    }
}


#if 1

static void testPumpData()
{
    WVCO<TestComposite> wvco;  

    wvco.init(); 
    wvco.inputs[WVCO<TestComposite>::MAIN_OUTPUT].channels = 8;
    wvco.inputs[WVCO<TestComposite>::VOCT_INPUT].channels = 8;
    wvco.params[WVCO<TestComposite>::WAVE_SHAPE_PARAM].value  = 0;

    wvco.step(); 
    float x = wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(0); 
    wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(1); 
    wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(2);
    wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(3); 
    wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(4);
    wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(5); 
    wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(6);
    wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(7);
 
    assertGE(x , -10);
    assertLE(x , 10); 
} 



/**
 * init func called first.
 * then run a while 
 * then call runFunc
 * then run real test
 */
static void testOutputLevels(int waveForm, float levelValue, float expectedLevel, 
    std::function<void(Comp&)> initFunc,
    std::function<void(Comp&)> runFunc)
{ 
  //  printf("test output levels()\n"); fflush(stdout);
    assertGE(waveForm, 0); 
    assertLE(waveForm, 2); 
    WVCO<TestComposite> wvco; 

    initComposite(wvco);
    wvco.inputs[WVCO<TestComposite>::MAIN_OUTPUT].channels = 1;
    wvco.inputs[WVCO<TestComposite>::VOCT_INPUT].channels = 1;
    wvco.params[WVCO<TestComposite>::OCTAVE_PARAM].value = 4;       // 7 was ok
    wvco.params[WVCO<TestComposite>::WAVE_SHAPE_PARAM].value  = waveForm;
    wvco.params[WVCO<TestComposite>::WAVESHAPE_GAIN_PARAM].value  = 50;
    wvco.params[WVCO<TestComposite>::OUTPUT_LEVEL_PARAM].value  = levelValue;
    if (initFunc) {
        initFunc(wvco);
    }
    assert(runFunc == nullptr);

 // printf("test output levels2, set output level to %f\n", levelValue); fflush(stdout);
    float positive = -100;
    float negative = 100; 
    float sum = 0; 
    const int iterations = 10000;  
    for (int i=0; i < iterations; ++i) {  
        wvco.step();
        float x = wvco.outputs[WVCO<TestComposite>::MAIN_OUTPUT].getVoltage(0); 
        sum += x;
        positive = std::max(positive, x); 
        negative = std::min(negative, x);  
    } 
 
    assertClose(positive, expectedLevel, 1);
    assertClose(negative, -expectedLevel, 1); 
    sum /= iterations;
    assertClose(sum, 0, .03);
}


static void testLevelControl()
{
    try {
    //expect 5 v with full level
    testOutputLevels(0, 100, 5, nullptr, nullptr); 

    //expect 5 v with full level
    testOutputLevels(0, 100, 5, nullptr, nullptr); 

    // expects zero when level off
    testOutputLevels(0, 0, 0, nullptr, nullptr); 
    } catch (std::exception& ex) {
 
        printf("exception, %s\n", ex.what());
    }
}


static void testOutputLevels() 
{
    testOutputLevels(0, 100, 5, nullptr, nullptr);
    testOutputLevels(1, 100, 5, nullptr, nullptr);
    testOutputLevels(2, 100, 5, nullptr, nullptr);
}

static void testEnvLevel()
{
    std::function<void(Comp&)> initFunc = [](Comp& comp) {
        comp.inputs[Comp::GATE_INPUT].setVoltage(0, 0);         // gate low
       // ADSR_OUTPUT_LEVEL_PARAM
        comp.params[Comp::ADSR_OUTPUT_LEVEL_PARAM].value = 1;   // adsr controls level
    };
    // no gate, expect no output. even with level all the ways
    testOutputLevels(0, 100, 0, initFunc, nullptr);
}


void testWVCO()
{
    testTriFormula();
    testTriFormula2();
    testPumpData();
    testADSR0();
    testADSR1();
    testADSR2();
    testADSR3();
    testADSRSingle();

    testOutputLevels();
    testLevelControl();
 
    testEnvLevel();
}

#endif
