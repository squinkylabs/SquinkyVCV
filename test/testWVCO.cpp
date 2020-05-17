
#include "TestComposite.h"
#include "WVCO.h"
#include "ADSR16.h"
#include "asserts.h"

static void testTriFormula()
{
    float a;
    float b;
    TriFormula::getLeftA(a, .5);
    assertEQ(a, 2);

    TriFormula::getLeftA(a, .1);
    assertEQ(a, 10);

    TriFormula::getRightAandB(a, b, .5);
    assertEQ(a, -2);
    assertEQ(b, 2);


}

static void testTriFormula2()
{
    for (float k = .1; k < 1; k += .09f) {
        float a, b;
        TriFormula::getLeftA(a, k);
        assertEQ(k * a, 1);

        TriFormula::getRightAandB(a, b, k);
        assertEQ(k * a + b, 1);
        assertEQ(1.f *a + b, 0);
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
   // high[0] = float_4::mask();


    printf("\n** step low **\n");
 
    for (int i=0; i<5; ++i) {
        printf("\nstep %d *****\n", i);
        adsr.step(gates, sampleTime);
    }

    printf("\n** step high **\n"); 

    gates[0] = float_4::mask(); 

    for (int i=0; i<5; ++i) {
        printf("\nstep %d *****\n", i);
        adsr.step(gates, sampleTime); 
    }
   
   // should have attacked a bit
    float_4 out = adsr.env[0]; 
    simd_assertGT(out, float_4(.1));  

}

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

void testWVCO()
{
    testTriFormula();
    testTriFormula2();
    testPumpData();
    testADSR0();
    testADSR1();
}