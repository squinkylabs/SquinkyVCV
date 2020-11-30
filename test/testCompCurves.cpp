
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


static void testLookupBelowThesh(float ratio, float kneeWidth)
{
   
    // Should be unity gain up to where it starts to bend
   
    float lowCornerDb = float(AudioMath::db(1)) - kneeWidth / 2;
    float lowKneeCornerVolts = float(AudioMath::gainFromDb(lowCornerDb));

   // float lowKneeCorner = normalizedThreshold - (kneeWidth / 2);
    // Comp ratio 1 is unity gain everywhere.
    CompCurves::Recipe r;
    r.ratio = ratio;

    auto table = CompCurves::makeCompGainLookup(r);    
    assertGT(table->size(), 0);
    float y = CompCurves::lookup(table, 0);
    assertEQ(y, 1);

    y = CompCurves::lookup(table, lowKneeCornerVolts);
    assertEQ(y, 1);
}

static void testLookupBelowTheshNoKnee()
{
    testLookupBelowThesh(1, 0);
    testLookupBelowThesh(4, 0);
}

static void testLookupBelowTheshSoftKnee()
{
    testLookupBelowThesh(1, 10);
    testLookupBelowThesh(4, 10);
}
static void validateCurve(CompCurves::LookupPtr table, CompCurves::Recipe r)
{

    bool first=true;
    float last=0;
    for (float x = .01f; x < 10; x += .01f) {
        const float y = CompCurves::lookup(table, x);
        assert(y <= 1);

        if (x < normalizedThreshold) {
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
    // comp ratio of 1 is a straight line - two points
    CompCurves::Recipe r;
    r.ratio = ratioToTest;

    auto table = CompCurves::makeCompGainLookup(r);

    assertGT(table->size(), 0);
    float y = CompCurves::lookup(table, normalizedThreshold);
    assertEQ(y, normalizedThreshold);

    const float dbChangeInInput = 20;       // arbitrary, let's pick 20 db
    const float voltChangeInInput = (float)AudioMath::gainFromDb(dbChangeInInput);

    const float expectedDbOutputAtMax = 20 / ratioToTest;

    const float expectedDbReductionAtMax = expectedDbOutputAtMax - dbChangeInInput;;


    // if the thrshold is 1, then we expect unity gain at 1
    // at 10 we are 20 louder
    assert(r.kneeWidth == 0);
    const float gain_y0 = CompCurves::lookup(table, normalizedThreshold);
    const float gain_y1 = CompCurves::lookup(table, normalizedThreshold * 10);
    assertEQ(gain_y0, normalizedThreshold);


    const double y1Db = AudioMath::db(gain_y1);
    const float observedDbReduction = float(y1Db);

    assertEQ(observedDbReduction, expectedDbReductionAtMax);

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

static void testLookupAboveTheshKnee()
{
    CompCurves::Recipe r;
    r.ratio = 4;
    r.kneeWidth = 6;            // make 6 db wide soft knee


    auto table = CompCurves::makeCompGainLookup(r);

    const float topOfKneeDb = r.kneeWidth / 2;
    const float topOfKneeVin = float(AudioMath::gainFromDb(topOfKneeDb));
    
    assertGT(table->size(), 0);
    float y = CompCurves::lookup(table, normalizedThreshold);
    assertEQ(y, normalizedThreshold);

    const float dbChangeInInput = 20;       // arbitrary, let's pick 20 db
    const float voltChangeInInput = (float)AudioMath::gainFromDb(dbChangeInInput);

    const float expectedDbOutputAtMax = 20 / r.ratio;

    const float expectedDbReductionAtMax = expectedDbOutputAtMax - dbChangeInInput;;


    // if the thrshold is 1, then we expect unity gain at 1
    // at 10 we are 20 louder
    assert(r.kneeWidth == 0);
    const float gain_y0 = CompCurves::lookup(table, normalizedThreshold);
    const float gain_y1 = CompCurves::lookup(table, normalizedThreshold * 10);
    assertEQ(gain_y0, normalizedThreshold);


    const double y1Db = AudioMath::db(gain_y1);
    const float observedDbReduction = float(y1Db);

    assertEQ(observedDbReduction, expectedDbReductionAtMax);

    validateCurve(table, r);
  
    assert(false);
}


static std::vector<float> generateGainVsInpuVoltageCurve(CompCurves::LookupPtr table, float x0, float x1, int numEntries)
{
    std::vector<float> v;
    assertGT( x1, x0);

    const float delta = (x1 - x0) / numEntries;
    for (float x = x0; x < x1; x += delta) {
        const float gain = CompCurves::lookup(table, x);
        v.push_back(gain);
    }
    if (v.size() > numEntries) {
        v.pop_back();
    }
    assertEQ(v.size(), numEntries);
    return v;
}

static std::vector<float> generateGainVsInpuDbCurve(CompCurves::LookupPtr table, float x0, float x1, int numEntries)
{
    std::vector<float> g;

    assertGT(x1, x0);

    const float dbMin = float(AudioMath::db(x0));
    const float dbMax = float(AudioMath::db(x1));
    const float delta = (dbMax - dbMin) / numEntries;
    assertGT(dbMax, dbMin);

    for (float dbIn = dbMin; dbIn <= dbMax; dbIn += delta) {
        float inputLevel = float(AudioMath::gainFromDb(dbIn));
        const double gain = CompCurves::lookup(table, inputLevel);
        const double vOut = inputLevel * gain;
        const float outputDb = float(AudioMath::db(vOut));
        g.push_back(float(gain));
    }
    if (g.size() > numEntries) {
        g.pop_back();
    }
    assertEQ(g.size(), numEntries);
    return g;
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
        const double vOut = inputLevel * gain;
        const float outputDb = float(AudioMath::db(vOut));
        db.push_back(outputDb);
    }
    if (db.size() > numEntries) {
        db.pop_back();
    }
    assertEQ(db.size(), numEntries);
    return db;
}

static void plotCurve(CompCurves::Recipe r, const std::string& fileName)
{

    auto table = CompCurves::makeCompGainLookup(r);

    FILE* fp = nullptr;
    fopen_s(&fp, fileName.c_str(), "w");

    const int tableSize = 40;
    auto vGain = generateGainVsInpuDbCurve(table, .1f, 10.f, tableSize);
    auto vDb = generateDbCurve(table, .1f, 10.f, tableSize);
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
    plotCurve(r, "curves-4-1-hard.csv");
}

static void plot4_1_soft()
{
    printf("\n------- soft curve -----\n");
    CompCurves::Recipe r;
    r.ratio = 4;
    r.kneeWidth = 10;
    printf("ratio=%f knee width = %f\n", r.ratio, r.kneeWidth);
    plotCurve(r, "curves-4-1-soft.csv");
    printf("----- end curve ----\n");
}

static void testCompCurvesKnee2()
{
    CompCurves::Recipe r;
    r.ratio = 4;
    r.kneeWidth = 10;

    auto table = CompCurves::makeCompGainLookup(r);
    assert(false);

}
    
static void testInflection()
{
    CompCurves::Recipe r;
    {
        r.ratio = 1;
        r.kneeWidth = 12;       // 12 db - 6 on each side

        // check for gain of 1 at inflection
        auto result = CompCurves::getGainAtLeftInflection(r);
        assertClose(result.x, .5, .002);
        assertClose(result.y, 1, .002);

        result = CompCurves::getGainAtRightInflection(r);
        assertClose(result.x, 2, .01);
        assertClose(result.y, 2, .01);
    }
    {
        r.ratio = 4;
        r.kneeWidth = 12;       // 12 db - 6 on each side

        // check for gain of 1 at inflection
        auto result = CompCurves::getGainAtLeftInflection(r);
        assertClose(result.x, .5, .002);
        assertClose(result.y, 1, .002);

        const float expectedGainAtRight = float(AudioMath::gainFromDb(6.0 / 4.0));
        result = CompCurves::getGainAtRightInflection(r);
        assertClose(result.x, 2, .01);
        assertClose(result.y, expectedGainAtRight, .01);
    }

}


// This one is more or less that same as testLookupAboveTheshNoKnee2,
// but it's a litte more clearly written, and has a larger range

static void testLookupAboveTheshNoKnee2(float ratioToTest)
{
    // comp ratio of 1 is a straight line - two points
    CompCurves::Recipe r;
    r.ratio = ratioToTest;

    auto table = CompCurves::makeCompGainLookup(r);

    assertGT(table->size(), 0);
    float y = CompCurves::lookup(table, normalizedThreshold);
    assertEQ(y, normalizedThreshold);

    const float threshDb = float(AudioMath::db(normalizedThreshold));

    for (float input = 2; input < 100; input *= 2) {
        const float inputDb = float(AudioMath::db(input));
        const float inputDbAboveTh = inputDb - threshDb;

        const float gain = CompCurves::lookup(table, input);
        const float output = input * gain;
        const float outputDb = float(AudioMath::db(output));

        const float outputDbAboveTh = outputDb - threshDb;

        const float observedRatio = inputDbAboveTh / outputDbAboveTh;
        assertClosePct(observedRatio, ratioToTest, 1);

    }
}


static void testLookupAboveTheshNoKnee2()
{
    testLookupAboveTheshNoKnee2(8);
}

void testCompCurves()
{
    testInflection();
   
   // testSpline();
    testLookupBelowTheshNoKnee();
    testLookupBelowTheshSoftKnee();

    testLookupAboveTheshNoKneeNoComp();
    testLookupAboveTheshNoKnee();
    testLookupAboveTheshNoKnee2();

    // TODO: make these test work
  //  testLookupAboveTheshKnee();
  //  testCompCurvesKnee2();
    // plot4_1_hard();
  //  plot4_1_soft();
}
