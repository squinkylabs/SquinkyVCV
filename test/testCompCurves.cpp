
#include "Cmprsr.h"
#include "CompCurves.h"
#include "SplineRenderer.h"
#include "asserts.h"

// At the moment, this doesn't test, just prints
#if 0
static void testSplineSub(HermiteSpline::point p0,
                          HermiteSpline::point p1,
                          HermiteSpline::point m0,
                          HermiteSpline::point m1) {
    // try to generate a section of limiter
    // the non-compress part is slope 1, and we try to carry that through
    HermiteSpline s(p0, p1, m0, m1);

    HermiteSpline::point last(0, 0);
    for (int i = 0; i < 11; ++i) {
        double t = i / 10.0;
        auto x = s.renderPoint(t);

        double slope = (x.second - last.second) / (x.first - last.first);
        printf("p[%d] = %f, %f (t=%f) slope = %f\n", i, x.first, x.second, t, slope);
        // last.x = x.first;
        //  last.y = x.second;
        last = x;
    }
}
#endif

static void characterizeSpline(float ratio, float t2y) {

    // final value and final slope?
    //double y2 = 6 * (-1.0 + 1.0 / ratio);
    const double desiredEndSlope = -1.0 + 1.0 / ratio;
    const double desiredEndValue = 6 * desiredEndSlope;
    
    const double secondX = 18;
    HermiteSpline h = {
        HermiteSpline::point(-6, 0),            // we know we start flat at zero
        HermiteSpline::point(6, desiredEndValue),            // and we leave where the hard curve would
        HermiteSpline::point(secondX, 0),             // tangent for start doesn't matter, as long is y values are the same
        HermiteSpline::point(6, t2y) };

    auto p0 = h.renderPoint(0);
    auto p1 = h.renderPoint(1);

    auto p9 = h.renderPoint(.9);
    auto p99 = h.renderPoint(.99);
    auto p999 = h.renderPoint(.999);


    SQINFO("\n\nfor ratio=%f, t2y=%f", ratio, t2y);
 //   SQINFO("y2 = %f lastV=%f", y2, y2 + 6 * deltaY);
    SQINFO("desired end value = %f, des slope=%f", desiredEndValue, desiredEndSlope);
    SQINFO("p0=%f,%f, p1=%f, %f", p0.first, p0.second, p1.first, p1.second);
    SQINFO("final slope99 = %f", (p1.second - p99.second) / (p1.first - p99.first));
    SQINFO("slope 9 = %f, 999=%f\n",
        (p1.second - p9.second) / (p1.first - p9.first),
        (p1.second - p999.second) / (p1.first - p999.first));
  
#if 0
    for (int i=0; i <= 10; ++i) {
        double t = double(i) / 10;
        auto p = h.renderPoint(t);
        SQINFO("i=%d x=%f, y=%f", i, p.first, p.second);
    }
#endif
    auto ptl = h.renderPoint(0);
    double lastSlope = 100;
    for (int i = 1; i <= 1000; ++i) {
        double t = double(i) / 1000;
        auto p = h.renderPoint(t);

        const double slope = (p.second - ptl.second) / (p.first - ptl.first);
        assert(slope <= lastSlope);
        ptl = p;
        lastSlope = slope;
        //SQINFO("i=%d x=%f, y=%f", i, p.first, p.second);
    }
}

// Not really a tests - used this to design the curves
// can probably turn it off?
static void testSpline() {

    // For second x=6 want value = -3 slope= -.5
    // -3 -> -.499
    characterizeSpline(2, -3.f);

    // For second x=6 want end = -4.5, slope = -.75
    // -4.5 -> -.748
    // For second x=12 
    // -4.5 -> -.749 ok
     // For second x=24 
    characterizeSpline(4, -4.5);  // 4:1


    // For second x=6 want value = -5.35, slope = -.875
    // -5.25 -> -.873 ok
    characterizeSpline(8, -5.25f);  // 8:1

    // wFor second x=6 want value = -5.7, slope = -.95
    // -5.75 -> -.956 ok
    characterizeSpline(20, -5.75f);  // 20:1
}

static void testLookupBelowThesh(float ratio, float kneeWidth, std::function<float(float)> lookup) {
    // Should be unity gain up to where it starts to bend
    float gainVatKneeBottom = 1;
    float inputLevelAtBottom = .5;  // 6 db below is where we kick int

    assertEQ(lookup(0), 1);
    assertEQ(lookup(inputLevelAtBottom), 1);
}

static std::vector<int> ratios = { 2, 4, 8, 20 };
static std::vector<CompCurves::Recipe> all = {
    {2, 0},
    {4, 0},
    {8, 0},
    {20, 0},
    {2, 12},
    {4, 12},
    {8, 12},
    {20, 12}
};

static void testLookupBelowThesh() {
   
    for (auto x : all) {
        testLookupBelowThesh(x.ratio, x.kneeWidth, CompCurves::getLambda(x, CompCurves::Type::ClassicLin));
        testLookupBelowThesh(x.ratio, x.kneeWidth, CompCurves::getLambda(x, CompCurves::Type::ClassicNU));
        testLookupBelowThesh(x.ratio, x.kneeWidth, CompCurves::getLambda(x, CompCurves::Type::SplineLin));
    }
}

static void testLookupAboveThesh(const CompCurves::Recipe& r, std::function<float(float)> lookup, double acceptedError) {

    // SQINFO("in test ratio=%f knee=%f", r.ratio, r.kneeWidth);
    const float topOfKneeDb = r.kneeWidth / 2;
    const float topOfKneeVin = float(AudioMath::gainFromDb(topOfKneeDb));

    const float dbChangeInInput = 20;  // arbitrary, let's pick 20 db
    const float voltChangeInInput = (float)AudioMath::gainFromDb(dbChangeInInput);


    const float vIn1 = 2;   //gain of 2 and we are over threshold, just barely
    const float vIn2 = vIn1 * voltChangeInInput;

    const float gain1 = lookup(vIn1);
    const float gain2 = lookup(vIn2);

    const float vOut1 = vIn1 * gain1;
    const float vOut2 = vIn2 * gain2;

    const double inputDbChange = AudioMath::db(vIn2) - AudioMath::db(vIn1);
    const double gainDbChange = AudioMath::db(gain2) - AudioMath::db(gain1);
    const double outputDbChange = AudioMath::db(vOut2) - AudioMath::db(vOut1);
    const double x = outputDbChange * r.ratio;
    assertClose(x, inputDbChange, acceptedError);
}

static void testLookupAboveThesh() {

    for (auto x : all) {
        testLookupAboveThesh(x, CompCurves::getLambda(x, CompCurves::Type::ClassicLin), .1);

        // don't care how bad classicNU is, althought we should stop using if for old comp
        testLookupAboveThesh(x, CompCurves::getLambda(x, CompCurves::Type::ClassicNU), 10.);
        testLookupAboveThesh(x, CompCurves::getLambda(x, CompCurves::Type::SplineLin), .1);
    }
}


#if 0
static void testLookupAboveTheshNoKnee(float ratioToTest) {
    // comp ratio of 1 is a straight line - two points
    CompCurves::Recipe r;
    r.ratio = ratioToTest;

    auto table = CompCurves::makeCompGainLookup(r);

    assertGT(table->size(), 0);
    float y = CompCurves::lookup(table, normalizedThreshold);
    assertEQ(y, normalizedThreshold);

    const float dbChangeInInput = 20;  // arbitrary, let's pick 20 db
    const float voltChangeInInput = (float)AudioMath::gainFromDb(dbChangeInInput);

    const float expectedDbOutputAtMax = 20 / ratioToTest;

    const float expectedDbReductionAtMax = expectedDbOutputAtMax - dbChangeInInput;
    ;

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

static void testLookupAboveTheshNoKneeNoComp() {
    testLookupAboveTheshNoKnee(1);
}

static void testLookupAboveTheshNoKnee() {
    testLookupAboveTheshNoKnee(2);
    testLookupAboveTheshNoKnee(4);
}

static void testLookupAboveTheshKnee() {
    CompCurves::Recipe r;
    r.ratio = 4;
    r.kneeWidth = 6;  // make 6 db wide soft knee

    auto table = CompCurves::makeCompGainLookup(r);

    const float topOfKneeDb = r.kneeWidth / 2;
    const float topOfKneeVin = float(AudioMath::gainFromDb(topOfKneeDb));

    assertGT(table->size(), 0);
    float y = CompCurves::lookup(table, normalizedThreshold);
    assertEQ(y, normalizedThreshold);

    const float dbChangeInInput = 20;  // arbitrary, let's pick 20 db
    const float voltChangeInInput = (float)AudioMath::gainFromDb(dbChangeInInput);

    const float expectedDbOutputAtMax = 20 / r.ratio;

    const float expectedDbReductionAtMax = expectedDbOutputAtMax - dbChangeInInput;
    ;

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
#endif

static std::vector<float> generateGainVsInpuVoltageCurve(CompCurves::LookupPtr table, float x0, float x1, int numEntries) {
    std::vector<float> v;
    assertGT(x1, x0);

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

static std::vector<float> generateGainVsInpuDbCurve(CompCurves::LookupPtr table, float x0, float x1, int numEntries) {
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

static std::vector<float> generateDbCurve(CompCurves::LookupPtr table, float x0, float x1, int numEntries) {
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

static void plotCurve(CompCurves::Recipe r, const std::string& fileName) {
    auto table = CompCurves::makeCompGainLookup(r);

    FILE* fp = nullptr;
#if _MSC_VER
    fopen_s(&fp, fileName.c_str(), "w");
#else
    fp = fopen(fileName.c_str(), "w");
#endif

    const int tableSize = 40;
    auto vGain = generateGainVsInpuDbCurve(table, .1f, 10.f, tableSize);
    auto vDb = generateDbCurve(table, .1f, 10.f, tableSize);
    assertEQ(vGain.size(), vDb.size());

    if (!fp) {
        printf("oops\n");
        return;
    }
    for (int i = 0; i < vGain.size(); ++i) {
        const float gain = vGain[i];
        const float dbOut = vDb[i];
        fprintf(fp, "%f, %f\n", gain, dbOut);
    }

    fclose(fp);
}

static void plot4_1_hard() {
    CompCurves::Recipe r;
    r.ratio = 4;
    plotCurve(r, "curves-4-1-hard.csv");
}

static void plot4_1_soft() {
    printf("\n------- soft curve -----\n");
    CompCurves::Recipe r;
    r.ratio = 4;
    r.kneeWidth = 10;
    printf("ratio=%f knee width = %f\n", r.ratio, r.kneeWidth);
    plotCurve(r, "curves-4-1-soft.csv");
    printf("----- end curve ----\n");
}

static void testCompCurvesKnee2() {
    CompCurves::Recipe r;
    r.ratio = 4;
    r.kneeWidth = 10;

    auto table = CompCurves::makeCompGainLookup(r);
    assert(false);
}

static void testInflection() {
    CompCurves::Recipe r;
    {
        r.ratio = 1;
        r.kneeWidth = 12;  // 12 db - 6 on each side

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
        r.kneeWidth = 12;  // 12 db - 6 on each side

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

static void testLookupAboveTheshNoKnee2(float ratioToTest) {
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

static void testLookupAboveTheshNoKnee2() {
    testLookupAboveTheshNoKnee2(8);
}

static void testContinuousCurveOld() {
    CompCurves::Recipe r;
    const float softKnee = 12;
    r.ratio = 4;
    r.kneeWidth = softKnee;
    CompCurves::LookupPtr lookup = CompCurves::makeCompGainLookup(r);
    auto cont = CompCurves::_getContinuousCurve(r, false);

    // const int k = 10000;
    const int k = 1000;
    for (int i = 0; i < k; ++i) {
        float x = (float)i / (k / 10);
        const float yLook = CompCurves::lookup(lookup, x);
        // SQINFO("x=%f yLook=%f", x, yLook);
        const float y = float(cont(x));
        assertClosePct(y, yLook, 1.f);
    }
}

static void testLookup2Old() {
    CompCurves::Recipe r;
    const float softKnee = 12;
    r.ratio = 4;
    CompCurves::CompCurveLookupPtr look = CompCurves::makeCompGainLookup2(r);
    assert(look);
    look->lookup(2.1099f);

    auto cont = CompCurves::_getContinuousCurve(r, false);
    const int k = 1000;
    for (int i = 0; i < k; ++i) {
        double x = (double)i / (k / 10);
        //const float yLook = CompCurves::lookup(lookup, x);
        const float yLook = look->lookup(float(x));
        // SQINFO("x=%f yLook=%f", x, yLook);
        const float y = float(cont(x));
        assertClosePct(yLook, y, 5.f);
    }
}

double getBiggestJump(double maxX, int divisions, std::function<double(double)> func) {
    //SQINFO("--- get biggest jump");
    double ret = 0;
    double lastValue = 1;
    double scaler = maxX / divisions;
    for (int i = 0; i < divisions; ++i) {
        //SQINFO("i = %d", i);
        double x = i * scaler;
        double y = func(x);
        assert(y <= 1);
        double dif = std::abs(y - lastValue);

        if (dif > ret) {
            // SQINFO("new max %f at x = %f", dif, x);
            ret = dif;
        }
        lastValue = y;
    }
    return ret;
}

double getBiggestSlopeJump(double maxX, int divisions, std::function<double(double)> func) {
    //SQINFO("--- get biggest jump");
    double ret = 0;
    double lastValue = 1;
    double lastSlope = 0;
    //  double lastSlopeDelta = 0;
    double scaler = maxX / divisions;
    for (int i = 0; i < divisions; ++i) {
        //SQINFO("i = %d", i);

        double x = i * scaler;
        double y = func(x);
        assert(y <= 1);
        // double slope = (y - lastValue) / scaler;
        double slope = 1;
        if (i > 0) {
            slope = (y - lastValue) / scaler;
        }
        if (i > 2) {
            double slope = (y - lastValue) / scaler;
            double slopeDelta = slope - lastSlope;

            // gain is always getting more and more reduced
            assert(slope <= 0);

            // It would be nice it slope were strictly reducing, but it isn't true with our lame quadratic knee.
            // assert(slope <= lastSlope);

            double dif = std::abs(slopeDelta - ret);
            // SQINFO("at x=%f y=%f slope=%f jump=%f", x, y, slope, dif);

            if (dif > ret) {
                SQINFO("new max slope change of %f at %f slope=%f was%f delta=%f", dif, x, slope, lastSlope, slopeDelta);
                ret = dif;
            }
        }
        lastValue = y;
        lastSlope = slope;
    }
    return ret;
}

static void testBiggestJumpOld() {
    const int div = 100003;
    CompCurves::Recipe r;
    const float softKnee = 12;
    r.ratio = 4;
    r.kneeWidth = softKnee;
    auto ref = CompCurves::_getContinuousCurve(r, false);
    double dRef = getBiggestJump(100, div, [ref](double x) {
        return ref(x);
    });

    auto oldCurve = CompCurves::makeCompGainLookup(r);
    double dRefOld = getBiggestJump(100, div, [oldCurve](double x) {
        return CompCurves::lookup(oldCurve, float(x));
    });

    auto newCurve = CompCurves::makeCompGainLookup2(r);
    double dRefNew = getBiggestJump(100, div, [newCurve](double x) {
        return newCurve->lookup(float(x));
    });

    assertClosePct(float(dRefOld), float(dRef), 10.0f);
    assertClosePct(float(dRefNew), float(dRef), 10.0f);
}

static void testBiggestSlopeJumpOld() {
    SQINFO("---- slope jump");
    const int div = 10000;
    CompCurves::Recipe r;
    const float softKnee = 12;
    r.ratio = 4;
    r.kneeWidth = softKnee;
    auto ref = CompCurves::_getContinuousCurve(r, false);
    double dRef = getBiggestSlopeJump(100, div, [ref](double x) {
        return ref(x);
    });
    // assert(false);
}

// look at the compression curve after knee.
// does it maintain all the way to 100?
static void testOldHighRatio(CompCurves::Type t, int ratio) {
    CompCurves::Recipe r;
    const float softKnee = 12;
    r.ratio = float(ratio);
    float expectedRatio = 1.f / r.ratio;
    
    auto func = CompCurves::getLambda(r, t);
   // auto oldCurve = CompCurves::makeCompGainLookup(r);
    for (int i=2; i <= 50; i *= 2) {
        float level1 = float(i);
        float level2 = 2 * float(i);
        
        float gain1 = func(level1);
        float gain2 = func(level2);

        float out1 = gain1 * level1;
        float out2 = gain2 * level2;

        double dbOut1 = AudioMath::db(out1);
        double dbOut2 = AudioMath::db(out2);


        double inputDbDiff = 6;
        double outputDbDiff = dbOut2 - dbOut1;
        double ratio = outputDbDiff / inputDbDiff;

        assertClosePct(ratio, expectedRatio, 6);
        
      //  SQINFO("\ni=%d, input1=%f input2=%f   dbOut1=%f, dbOut2=%f ratio =%f", i, level1, level2, dbOut1, dbOut2, ratio);
        

    }
}

static void testOldHighRatio() {
    testOldHighRatio(CompCurves::Type::ClassicNU, 4);
    testOldHighRatio(CompCurves::Type::ClassicNU, 8);
    testOldHighRatio(CompCurves::Type::ClassicNU, 20);

    testOldHighRatio(CompCurves::Type::ClassicLin, 4);
    testOldHighRatio(CompCurves::Type::ClassicLin, 8);
    testOldHighRatio(CompCurves::Type::ClassicLin, 20);
}

// trival test of knee cureve. It should always reduce gain,
// and more reduciton the more input.
static void testSplineDecreasing() {
    auto h = HermiteSpline::make(4, 12);
    double lastGain = 5;
    for (int i=0; i<=10; ++i) {
        double t = double(i) / 10.0;
        auto pt = h->renderPoint(t);
        double gainDb = pt.second;
        assertLE(gainDb, 0);
        assertLT(gainDb, lastGain);
        lastGain = gainDb;
    }
}

static void testSplineVSOld() {
    CompCurves::Recipe r;
    // const float softKnee = 12;
    r.ratio = 4;
    r.kneeWidth = 12;
    auto old = CompCurves::_getContinuousCurve(r, false);
    auto spline = CompCurves::_getContinuousCurve(r, true);
    for (int i = 0; i < 10000; ++i) {
        double x = (double)i / 100.0;
        float yOld = float(old(x));
        float ySpline = float(spline(x));
        assertClosePct(ySpline, yOld, 5);
    }
}

#if 0
static double slope(double x1, double y1, double x2, double y2) {
    return (y2 - y1) / (x2 - x1);
}
#endif

static double slopeDB(double _x1, double _y1, double _x2, double _y2) {
    const double x1 = AudioMath::db(_x1);
    const double x2 = AudioMath::db(_x2);
    const double y1 = AudioMath::db(_y1);
    const double y2 = AudioMath::db(_y2);

    return (y2 - y1) / (x2 - x1);
}

static void testEndSlopeHardKnee(int ratio) {
    CompCurves::Recipe r;
    r.ratio = float(ratio);

    // use the classic lookup from Comp (1)
    CompCurves::LookupPtr lookup = CompCurves::makeCompGainLookup(r);

    const float xLow = .5f;
    const float xHigh = 1.01f;
    const float xHigh2 = float(1 + ratio);

    // lookup the gains at the sample points
    const float yGainLow = CompCurves::lookup(lookup, xLow);
    const float yGain1 = CompCurves::lookup(lookup, 1);
    const float yGainHigh = CompCurves::lookup(lookup, xHigh);
    const float yGainHigh2 = CompCurves::lookup(lookup, xHigh2);

    // convert gains to output levels
    const float y1 = yGain1 * 1;
    const float yLow = yGainLow * xLow;
    const float yHigh = yGainHigh * xHigh;
    const float yHigh2 = yGainHigh2 * xHigh2;

    // compressor ratio is slope of output / input in db
    const double slopeL = slopeDB(xLow, yLow, 1, y1);
    const double slopeH = slopeDB(1, y1, xHigh, yHigh);
    const double slopeH2 = slopeDB(1, y1, xHigh2, yHigh2);

    assertEQ(y1, 1);
    assertEQ(slopeL, 1);
    assertClosePct(slopeH2, 1.f / float(ratio), 15);
}

static void testEndSlopeSoftKnee(int ratio) {
    CompCurves::Recipe r;
    r.kneeWidth = 12;
    r.ratio = float(ratio);

    // use the classic lookup from Comp (1)
    CompCurves::LookupPtr lookup = CompCurves::makeCompGainLookup(r);

    const float xLow1 = .1f;
    const float xLow2 = .5f;
    const float xHigh1 = 2.f;
    const float xHigh2 = xHigh1 + ratio;

    // lookup the gains at the sample points
    const float yGainLow1 = CompCurves::lookup(lookup, xLow1);
    const float yGainLow2 = CompCurves::lookup(lookup, xLow2);
    //const float yGain1 = CompCurves::lookup(lookup, 1);
    const float yGainHigh1 = CompCurves::lookup(lookup, xHigh1);
    const float yGainHigh2 = CompCurves::lookup(lookup, xHigh2);

    // convert gains to output levels
    const float yLow1 = yGainLow1 * xLow1;
    const float yLow2 = yGainLow2 * xLow2;

    const float yHigh1 = yGainHigh1 * xHigh1;
    const float yHigh2 = yGainHigh2 * xHigh2;

    // compressor ratio is slope of output / input in db
    const double slopeL = slopeDB(xLow1, yLow1, xLow2, yLow2);
    const double slopeH = slopeDB(xHigh1, yHigh1, xHigh2, yHigh2);
    // const double slopeH2 = slopeDB(1, y1, xHigh2, yHigh2);

    //  assertEQ(y1, 1.f);
    assertEQ(slopeL, 1);
    assertClosePct(slopeH, 1.f / float(ratio), 5);
}

static void testKneeSlope(int ratio, bool newCurve) {
    CompCurves::Recipe r;
    r.kneeWidth = 12;
    r.ratio = float(ratio);

    // use the classic lookup from Comp (1)
    CompCurves::LookupPtr lookup = CompCurves::makeCompGainLookup(r);
    std::shared_ptr<NonUniformLookupTableParams<double>> splineLookup = CompCurves::makeSplineMiddle(r);

    const float delta = .05f;
    const float xLow2 = .5f;
    const float xLow1 = .5f + delta;

    const float xHigh1 = 2.f - delta;
    const float xHigh2 = 2.f + ratio;

    // lookup the gains at the sample points
    float yGainLow1;
    float yGainLow2;
    float yGainHigh1;
    float yGainHigh2;
    if (!newCurve) {
        yGainLow1 = CompCurves::lookup(lookup, xLow1);
        yGainLow2 = CompCurves::lookup(lookup, xLow2);
        yGainHigh1 = CompCurves::lookup(lookup, xHigh1);
        yGainHigh2 = CompCurves::lookup(lookup, xHigh2);
    } else {
        yGainLow1 = (float)NonUniformLookupTable<double>::lookup(*splineLookup, xLow1);
        yGainLow2 = (float)NonUniformLookupTable<double>::lookup(*splineLookup, xLow2);
        yGainHigh1 = (float)NonUniformLookupTable<double>::lookup(*splineLookup, xHigh1);
        yGainHigh2 = (float)NonUniformLookupTable<double>::lookup(*splineLookup, xHigh2);
    }

    // convert gains to output levels
    const float yLow1 = yGainLow1 * xLow1;
    const float yLow2 = yGainLow2 * xLow2;

    const float yHigh1 = yGainHigh1 * xHigh1;
    const float yHigh2 = yGainHigh2 * xHigh2;

    // compressor ratio is slope of output / input in db
    const double slopeL = slopeDB(xLow1, yLow1, xLow2, yLow2);
    const double slopeH = slopeDB(xHigh1, yHigh1, xHigh2, yHigh2);

    assertClosePct(slopeL, .97f, 5);
    assertClosePct(slopeH, 1.f / float(ratio), 5);
}

static void testEndSlopeHardKnee() {
    testEndSlopeHardKnee(4);
    testEndSlopeHardKnee(8);
    testEndSlopeHardKnee(20);
}

static void testEndSlopeSoftKnee() {
    testEndSlopeSoftKnee(4);
    testEndSlopeSoftKnee(8);
    testEndSlopeSoftKnee(20);
}

static void testKneeSlope() {
    testKneeSlope(4, false);
    testKneeSlope(8, false);
    testKneeSlope(20, false);

    SQWARN("-- do this test for new spline. very important");
#if 0
    testKneeSlope(4, true);
    testKneeSlope(8, true);
    testKneeSlope(20, true);
#endif
}

static void testKneeSpline0(int ratio) {
    CompCurves::Recipe r;
    r.kneeWidth = 12;
    r.ratio = float(ratio);
    std::shared_ptr<NonUniformLookupTableParams<double>> splineLookup = CompCurves::makeSplineMiddle(r); 

    // the soft knee needs to match up with the hard knee,
    // so this simple formula works
    const double desiredEndSlopeDb = -1.0 + 1.0 / ratio;
    const double desiredEndValueDb = 6 * desiredEndSlopeDb;

    // now take out of Db space and convert to linear gain
    const float desiredEndGainVolts = (float) AudioMath::gainFromDb(desiredEndValueDb);

    const float y0 = (float) NonUniformLookupTable<double>::lookup(*splineLookup, .5);
    const float y1 = (float) NonUniformLookupTable<double>::lookup(*splineLookup, 2);

    const float expectedFinalY_ = 1.0f + 1.0f / r.ratio;
    assertClose(y0, 1.f, .001);     // I think the prev .5 was wrong. unity gain is what we want
    assertClosePct(y1, desiredEndGainVolts, 1);
}

static void testKneeSpline0() {
    testKneeSpline0(4);
    testKneeSpline0(8);
    testKneeSpline0(20);
}

static void testBasicSplineImp() {
    CompCurves::Recipe r;
    r.kneeWidth = 12;
    r.ratio = 4;
    std::shared_ptr<NonUniformLookupTableParams<double>> splineLookup = CompCurves::makeSplineMiddle(r);

    const float expectedY2 = 1.0f + 1.0f / r.ratio;
    SQINFO("expecting y=%f at 2\n", expectedY2);
    const float div = 20;
    for (int i = 0; i <= div; ++i) {
        float x = .5f + 1.5f * float(i) / div;

        float y = (float) NonUniformLookupTable<double>::lookup(*splineLookup, x);
        SQINFO("x=%f y=%f", x, y);
    }
  //  assert(false);
}




static void testMake3() {
    CompCurves::Recipe r;
    CompCurves::makeCompGainLookup3(r) ;
}

void testCompCurves() {
    Cmprsr::_reset();
    assertEQ(_numLookupParams, 0);
    testInflection();

    testSpline();
    testLookupBelowThesh();

    testLookupAboveThesh();
  //  testLookupAboveTheshNoKneeNoComp();
  //  testLookupAboveTheshNoKnee();
  //  testLookupAboveTheshNoKnee2();

    // TODO: make these test work
    //  testLookupAboveTheshKnee();
    //  testCompCurvesKnee2();
    // plot4_1_hard();
    //  plot4_1_soft();
    testContinuousCurveOld();
    testLookup2Old();
    testBiggestJumpOld();
    testBiggestSlopeJumpOld();
    testOldHighRatio();

    testSplineDecreasing();
    testKneeSpline0();
    testBasicSplineImp();
    testEndSlopeHardKnee();
    testEndSlopeSoftKnee();
    testKneeSlope();
    // testSplineVSOld();
    testMake3();
    assertEQ(_numLookupParams, 0);
}
