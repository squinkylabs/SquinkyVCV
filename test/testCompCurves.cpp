
#include "CompCurves.h"
#include "SplineRenderer.h"
#include "asserts.h"

// At the moment, this doesn't test, just prints
static void testSpline()
{
    // try to generate a section of limiter
    // the non-compress part is slope 1, and we try to carry that through
    HermiteSpline s(
        HermiteSpline::point(0, 0),         // p0
        HermiteSpline::point(1, .5),        // p1
        HermiteSpline::point(1, 1),         // m0 (p0 out)
        HermiteSpline::point(0, .5)         // m1 (p1 in)
    );

    HermiteSpline::point last(0, 0);
    for (int i=0; i<11; ++i) {
        double t = i / 10.0;
        auto x = s.renderPoint(t);

        double slope = (x.second - last.second) / (x.first - last.first);
        printf("p[%d] = %f, %f (t=%f) slope = %f\n", i, x.first, x.second, t, slope);
       // last.x = x.first;
      //  last.y = x.second;
        last = x;
    }
  //  abort();
}

static void testLookupBelowTheshNoKnee()
{
    // comp ratio of 1 is a straight line - two points
    CompCurves::Recipe r;
  //  r.minX = -10;
  //  r.maxX = 10;
    r.ratio = 1;
    r.threshold = 1;

    auto table = CompCurves::makeCompGainLookup(r);
    
    assertGT(table->size(), 0);

    printf("table size = %d\n", table->size());

    float y = CompCurves::lookup(table, 0);
    assertEQ(y, 0);

    y = CompCurves::lookup(table, r.threshold);
    assertEQ(y, r.threshold);
}

void foo()
{
    float thresh = 1;
    float dbTh = 2;     // arbitrary
  //  float gTh = AudioMath::gainFromDb(dbTh);

    float db10Th = 0;
}

static void testLookupAboveTheshNoKneeNoComp()
{
    // comp ratio of 1 is a straight line - two points
    CompCurves::Recipe r;
  //  r.minX = -10;
 //   r.maxX = 10;
    r.ratio = 1;
    r.threshold = 1;

    auto table = CompCurves::makeCompGainLookup(r);
    
    assertGT(table->size(), 0);
    float y = CompCurves::lookup(table, r.threshold);
    assertEQ(y, r.threshold);

    const float dbThresh = 0;           // unity again a thresh if no knee
    const double dbThresh10 = AudioMath::db(10);   // input 10 X thresh

    y = CompCurves::lookup(table, 10);
    assertEQ(y, 1);     // ratio of 1 is constant x1 gain
}

void testOldStuff();
void testCompCurves()
{
   // testSpline();
    testOldStuff();
    testLookupBelowTheshNoKnee();
    testLookupAboveTheshNoKneeNoComp();
  
    // testCompCurvesKnee2();
}

//***************** tests for deprecated func **********
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

static void testOldStuff()
{
    testGainFuncNoKnee();
    testCompCurves1();
    testCompCurves2();
}

