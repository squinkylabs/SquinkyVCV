
#include "CompCurves.h"
#include "SplineRenderer.h"
#include "asserts.h"

static void testSpline()
{
    // try info compression
    HermiteSpline s(
        HermiteSpline::point(0, 0),     // p0
        HermiteSpline::point(1, .5),     // p1
        HermiteSpline::point(1, 1),     // m0 (p0 out)
        HermiteSpline::point(0, .5)      // m1 (p1 in)
    );
    for (int i=0; i<11; ++i) {
        double t = i / 10.0;
        auto x = s.renderPoint(t);
        printf("p[%d] = %f, %f (t=%f)\n", i, x.first, x.second, t);
    }
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
    // testCompCurvesKnee2();
}