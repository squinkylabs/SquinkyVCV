
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
    r.ratio = 1;
    r.threshold = 1;

    auto table = CompCurves::makeCompGainLookup(r);    
    assertGT(table->size(), 0);
    float y = CompCurves::lookup(table, 0);
    assertEQ(y, 0);

    y = CompCurves::lookup(table, r.threshold);
    assertEQ(y, r.threshold);
}



#if 0 // worked got 1:1
static void testLookupAboveTheshNoKnee(float ratioToTest)
{
    printf("enter test for ratio = %f\n", ratioToTest);
    // comp ratio of 1 is a straight line - two points
    CompCurves::Recipe r;
    r.ratio = ratioToTest;
    r.threshold = 1;

    auto table = CompCurves::makeCompGainLookup(r);
    
    assertGT(table->size(), 0);
    float y = CompCurves::lookup(table, r.threshold);
    assertEQ(y, r.threshold);

  // const float dbThresh = 0;           // unity again a thresh if no knee
  //  const double dbThresh10 = AudioMath::db(10);   // input 10 X thresh

    // if the thrshold is 1, then we exect unity gain at 1
    // at 10 we are 20 louder
    float expectedGain = (float) AudioMath::gainFromDb(20 / ratioToTest);
    if (ratioToTest == 1) assertEQ(expectedGain, 1);

    y = CompCurves::lookup(table, 10);
    assertEQ(y, expectedGain);     // ratio of 1 is constant x1 gain
}
#endif

static void testLookupAboveTheshNoKnee(float ratioToTest)
{
    printf("\n***** enter test for ratio = %f\n", ratioToTest);
    // comp ratio of 1 is a straight line - two points
    CompCurves::Recipe r;
    r.ratio = ratioToTest;
    r.threshold = 1;

    auto table = CompCurves::makeCompGainLookup(r);
    
    assertGT(table->size(), 0);
    float y = CompCurves::lookup(table, r.threshold);
    assertEQ(y, r.threshold);

    const float dbChangeInInput = 20;       // arbitrary, let's pick 20 db
    const float voltChangeInInput = (float)AudioMath::gainFromDb(dbChangeInInput);

    const float expectedDbOutputAtMax = 20 / ratioToTest;

   // const float expectedDbReductionAtMax = dbChangeInInput - expectedDbOutputAtMax;
    const float expectedDbReductionAtMax = expectedDbOutputAtMax - dbChangeInInput;;

  // const float dbThresh = 0;           // unity again a thresh if no knee
  //  const double dbThresh10 = AudioMath::db(10);   // input 10 X thresh

    // if the thrshold is 1, then we exect unity gain at 1
    // at 10 we are 20 louder
    assert(r.kneeWidth == 0);
    const float gain_y0 = CompCurves::lookup(table, r.threshold);
    const float gain_y1 = CompCurves::lookup(table, r.threshold * 10);
    assertEQ(gain_y0, r.threshold);


    const double y1Db = AudioMath::db(gain_y1);
    const float observedDbReduction = float(y1Db);

    printf("ex=%f, obs = %f\n", expectedDbReductionAtMax, observedDbReduction);
    assertEQ(observedDbReduction, expectedDbReductionAtMax)
  //  assert(false);      // now what?
    
    // difference in gain at t * 10 relative to t.
 //   float deltaYDb = float(y1Db - y0Db);
 //   printf("with ratio %f seeing deltaYdb = %f\n", ratioToTest, deltaYDb);
  //  printf("g0=%f, g1=%f, y0db=%f 1=%f\n", gain_y0, gain_y1, y0Db, y1Db);
   // assertEQ(deltaYDb, 20 / ratioToTest);
   
  
}

static void testLookupAboveTheshNoKneeNoComp()
{
    testLookupAboveTheshNoKnee(1);
}

static void testLookupAboveTheshNoKnee()
{
    testLookupAboveTheshNoKnee(2);
    testLookupAboveTheshNoKnee(4);
}



void testOldStuff();
void testCompCurves()
{
   // testSpline();
    testOldStuff();
    testLookupBelowTheshNoKnee();
    testLookupAboveTheshNoKneeNoComp();
    testLookupAboveTheshNoKnee();
  
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

