
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

static void testLookupBelowTheshNoKnee(float thresh, float ratio)
{
    // Comp ratio 1 is unity gain everywhere.
    CompCurves::Recipe r;
    r.ratio = ratio;
    r.threshold = thresh;

    auto table = CompCurves::makeCompGainLookup(r);    
    assertGT(table->size(), 0);
    float y = CompCurves::lookup(table, 0);
    assertEQ(y, 1);

    y = CompCurves::lookup(table, r.threshold);
    assertEQ(y, 1);
}

static void testLookupBelowTheshNoKnee()
{
    testLookupBelowTheshNoKnee(1, 1);
    testLookupBelowTheshNoKnee(.1f, 4);
}

static void validateCurve(CompCurves::LookupPtr table, CompCurves::Recipe r)
{

    bool first=true;
    float last=0;
    for (float x = .01f; x < 10; x += .01f) {
        const float y = CompCurves::lookup(table, x);
        assert(y <= 1);

        if (x < r.threshold) {
            assert(r.kneeWidth == 0);   // don't know how to do this yet
            assertEQ(y, 1);             // unity gain below thresh
        }
        if (first) {
            first = false;
        } else {
            assert(y <= last);
        }
        last = y;
    }
}

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
    assertEQ(observedDbReduction, expectedDbReductionAtMax);
  //  assert(false);      // now what?
    
    // difference in gain at t * 10 relative to t.
 //   float deltaYDb = float(y1Db - y0Db);
 //   printf("with ratio %f seeing deltaYdb = %f\n", ratioToTest, deltaYDb);
  //  printf("g0=%f, g1=%f, y0db=%f 1=%f\n", gain_y0, gain_y1, y0Db, y1Db);
   // assertEQ(deltaYDb, 20 / ratioToTest);

    validateCurve(table, r);
   
  
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


static std::vector<float> generateGainCurve(CompCurves::LookupPtr table, float x0, float x1, int numEntries)
{
    std::vector<float> v;
    assertGT( x1, x0);

    const float delta = (x1 - x0) / numEntries;
    for (float x = x0; x < x1; x += delta) {
        const float gain = CompCurves::lookup(table, x);
       // const float gain = 1;
        v.push_back(gain);
        printf("in gain llop x=%f, gain=%f\n", x, gain);
    }
    if (v.size() > numEntries) {
        v.pop_back();
    }
    assertEQ(v.size(), numEntries);
    return v;
}

static std::vector<float> generateDbCurve(CompCurves::LookupPtr table, float x0, float x1, int numEntries)
{
    std::vector<float> db;

    assertGT(x1, x0);

    const float dbMin = float(AudioMath::db(x0));
    const float dbMax = float(AudioMath::db(x1));
    const float delta = (dbMax - dbMin) / numEntries;
    assertGT(dbMax, dbMin);


    for (float dbIn = dbMin; dbIn <= dbMax; dbIn += delta) {
        float inputLevel = float(AudioMath::gainFromDb(dbIn));
        const double gain = CompCurves::lookup(table, inputLevel);
        //const float gain = 1;
        const double vOut = inputLevel * gain;
        const float outputDb = float(AudioMath::db(vOut));
        printf("in db loop dbIn = %f, inputLevel = %f gain = %f output db = %f\n", dbIn, inputLevel, gain, outputDb);
        db.push_back(outputDb);
    }
    assertEQ(db.size(), numEntries);
    return db;
}

static void plotCurve(CompCurves::Recipe r, const std::string& fileName)
{
    auto table = CompCurves::makeCompGainLookup(r);

    FILE* fp = nullptr;
    fopen_s(&fp, fileName.c_str(), "w");

    const int tableSize = 20;
    auto vGain = generateGainCurve(table, .1f, 3.f, tableSize);
    auto vDb = generateDbCurve(table, .1f, 6.f, tableSize);
    printf("gain table has %zd\n", vGain.size());
    printf("db table has %zd\n", vDb.size());
    assertEQ(vGain.size(), vDb.size());


    if (!fp) {
        printf("oops\n");
        return;
    }
    for (int i = 0; i< vGain.size(); ++i) {
        const float gain = vGain[i];
        const float dbOut = vDb[i];
        fprintf(fp, "%f, %f\n", gain, dbOut);
    }
   
    fclose(fp);
}

static void plot4_1_hard()
{
    CompCurves::Recipe r;
    r.ratio = 4;
    r.threshold = 1;
    plotCurve(r, "curves-4-1-hard.csv");
}

#if 0
static void plotBasicCurve(float ratioToTest)
{
    CompCurves::Recipe r;
    r.ratio = ratioToTest;
    r.threshold = 1;
    auto table = CompCurves::makeCompGainLookup(r);


    FILE* fp = nullptr;
    fopen_s(&fp, "comp-curve.csv", "w");


    for (float x = .1f; x < 3; x += .1f) {
        const float y = CompCurves::lookup(table, x);
        fprintf(fp, "%f\n", y);
        
    }
    fclose(fp);
}
#endif
    

void testCompCurves()
{
   // testSpline();
    testLookupBelowTheshNoKnee();
    testLookupAboveTheshNoKneeNoComp();
    testLookupAboveTheshNoKnee();
  
    // testCompCurvesKnee2();
    plot4_1_hard();
}
