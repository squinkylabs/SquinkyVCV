
#include <map>
#include <vector>
#include "asserts.h"
#include "LookupTable.h"
#include "AsymWaveShaper.h"
#include "Shaper.h"
#include "TestComposite.h"
#include "TestSignal.h"


using Spline = std::vector< std::pair<double, double> >;



static void gen()
{
    for (int i = 0; i < AsymWaveShaper::iSymmetryTables; ++i) {
        float symmetry = float(i) / float(AsymWaveShaper::iSymmetryTables - 1);
        AsymWaveShaper::genTable(i, symmetry);
    }
}


static void testLook0()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 2);
    l.add(3, 3);

    double x = l.lookup(2.5);
    assertClose(x, 2.5, .0001);
}

static void testLook1()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 4);
    l.add(3, 3);

    double x = l.lookup(1.5);
    assertClose(x, 2.5, .0001);
}

static void testLook2()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 2);
    l.add(3, 3);

    double x = l.lookup(.5);
    assertClose(x, .5, .0001);
}

static void testLook3()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 2);
    l.add(3, 3);

    double x = l.lookup(2.5);
    assertClose(x, 2.5, .0001);
}

static void testLook4()
{
    NonUniformLookup l;
    l.add(0, 0);
    l.add(1, 1);
    l.add(2, 2);
    l.add(3, 3);

    double x = l.lookup(0);
    assertClose(x, 0, .0001);
}


static void testGen0()
{
//    AsymGenerator g(0, .001f);
    AsymWaveShaper g;
    const int index = 3;// symmetry
    float x = g.lookup(0, index);
    printf("0 ret = %f\n", x);

    x = g.lookup(1, index);
    printf("1 ret = %f\n", x);

    x = g.lookup(-1, index);
    printf("-1 ret = %f\n", x);

}

static void testDerivativeSub(int index, float delta)
{
    AsymWaveShaper ws;

    const float ya = ws.lookup(-delta, index);
    const float y0 = ws.lookup(0, index);
    const float yb = ws.lookup(delta, index);
    const float slopeLeft = -ya / delta;
    const float slopeRight = yb / delta;

  // printf("[%d] y0 = %f, slope left = %f, right =%f\n", index, y0, slopeLeft, slopeRight);

   // since I changed AsymWaveShaper to be points-1 this is worse
    assertClose(y0, 0, .00001);
    assertClose(slopeRight, 2, .01);
    if (index != 0) {
        assertClose(slopeLeft, 2, .3
        );
    }

}
static void testDerivative()
{
    // 6 with .1
    for (int i = AsymWaveShaper::iSymmetryTables - 1; i >= 0; --i) {
        testDerivativeSub(i, .0001f);
    }
}



static void testShaper0()
{
    Shaper<TestComposite> gmr;

    int shapeMax = (int) Shaper<TestComposite>::Shapes::Invalid;
    for (int i = 0; i < shapeMax; ++i) {
        gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) i;
        std::string s = gmr.getString(Shaper<TestComposite>::Shapes(i));
        assertGT(s.length(), 0);
        assertLT(s.length(), 20);
        gmr.params[Shaper<TestComposite>::PARAM_SYMMETRY].value = 0;
        for (int i = 0; i < 50; ++i) gmr.step();
        gmr.params[Shaper<TestComposite>::PARAM_SYMMETRY].value = 1;
        for (int i = 0; i < 50; ++i) gmr.step();
    }
}

static void testShaper1Sub(int shape, float gain, float targetRMS)
{
    Shaper<TestComposite> gmr;
    gmr.params[Shaper<TestComposite>::PARAM_SHAPE].value = (float) shape;
    gmr.params[Shaper<TestComposite>::PARAM_GAIN].value = gain;        // max gain
    const int buffSize = 1 * 1024;
    float buffer[buffSize];

    TestSignal<float>::generateSin(buffer, buffSize, 1.f / 40);
    double rms = TestSignal<float>::getRMS(buffer, buffSize);
    //printf("signal=%f\n", rms);
    for (int i = 0; i < buffSize; ++i) {
        const float x = buffer[i];
        gmr.inputs[Shaper<TestComposite>::INPUT_AUDIO].value = buffer[i];
        gmr.step();
        buffer[i] = gmr.outputs[Shaper<TestComposite>::OUTPUT_AUDIO].value;
    }
    rms = TestSignal<float>::getRMS(buffer, buffSize);
  //  const float targetRMS = 5 * .707f;

    const char* p = gmr.getString(Shaper<TestComposite>::Shapes(shape));
   printf("rms[%s] = %f target = %f ratio=%f\n", p, rms, targetRMS, targetRMS / rms);

   if (targetRMS > .01) {
       assertClose(rms, targetRMS, .5);
   }

}

static void testShaper1()
{
    int shapeMax = (int) Shaper<TestComposite>::Shapes::Invalid;
    for (int i = 0; i < shapeMax; ++i) {
        testShaper1Sub(i, 5, 5 * .707f);
        testShaper1Sub(i, 0, 0);

    }
}

void testSpline(bool doEmit)
{
    if (doEmit) {
        gen();
        return;
    }
    testLook0();
    testLook1();
    testLook2();
    testLook3();
    testLook4();
    testGen0();
    testDerivative();
    testShaper0();
    testShaper1();
}

