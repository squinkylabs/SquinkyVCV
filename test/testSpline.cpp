
#include <map>
#include <vector>
#include "asserts.h"
#include "LookupTable.h"
#include "AsymWaveShaper.h"


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

void testSpline()
{
    testLook0();
    testLook1();
    testLook2();
    testLook3();
    testLook4();
   // gen();
    testGen0();
}

