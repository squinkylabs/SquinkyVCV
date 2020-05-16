
#include "TestComposite.h"
#include "WVCO.h"
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
}