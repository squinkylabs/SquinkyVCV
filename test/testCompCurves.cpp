
#include "CompCurves.h"
#include "SplineRenderer.h"
#include "asserts.h"

static void testSpline()
{
    HermiteSpline s;
    s.renderPoint(0);
}

static void verifyCurve(const std::vector<CompCurves::xy>& curve)
{

    bool first = true;
    CompCurves::xy lastOne;
    for (auto q : curve)
    {
        if (first) {
            first = false;
        }
        else {
            assertGT(q.x, lastOne.x);
            assertGT(q.y, lastOne.y);
        }
        lastOne = q;
    }
}

static void  testGainFuncNoKnee()
{
    CompCurves::Recipe r;
    r.minX = -10;
    r.maxX = 10;
    r.ratio = 2;
    r.threshold = 0;

    auto func = CompCurves::continuousGainFunction(r);


    assertEQ(func(-10), -10);
    assertEQ(func(0), 0);
    assertEQ(func(.01f), .005f);
    assertEQ(func(10), 5);
}

static void testCompCurves1()
{
    // comp ratio of 1 is a straight line - two points
    CompCurves::Recipe r;
    r.minX = -10;
    r.maxX = 10;
    r.ratio = 1;
    r.threshold = 0;

    auto result = CompCurves::makeCrudeCompGainTable(r);
    assertEQ(result.size(), 3);

    verifyCurve(result);
}

static void testCompCurves2()
{
    CompCurves::Recipe r;
    r.minX = -10;
    r.maxX = 10;
    r.ratio = 2;
    r.threshold = 0;

    auto result = CompCurves::makeCrudeCompGainTable(r);

    assertEQ(result.size(), 3);
    verifyCurve(result);
}

static void testCompCurvesKnee2()
{
    CompCurves::Recipe r;
    r.minX = -10;
    r.maxX = 10;
    r.ratio = 2;
    r.threshold = 0;
    r.kneeWidth = 1;

    auto result = CompCurves::makeCrudeCompGainTable(r);

    assertGT(result.size(), 4);
    verifyCurve(result);
    assert(false);          // more tests
}

void testCompCurves()
{
    testSpline();
    testGainFuncNoKnee();
    testCompCurves1();
    testCompCurves2();
    testCompCurvesKnee2();
}