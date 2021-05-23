

#include "CompCurves.h"

#include <functional>

#include "SplineRenderer.h"
#include "asserts.h"

CompCurves::xy CompCurves::getGainAtRightInflection(const CompCurves::Recipe& r) {
    assert(r.ratio > 0);
    // on the right side, the straight part after inflection is just
    // output.db = input.db / ratio

    const float topOfKneeDb = r.kneeWidth / 2;
    const float topOfKneeVin = float(AudioMath::gainFromDb(topOfKneeDb));
    const float dbGainAtTopOfKnee = topOfKneeDb / r.ratio;
    const float gainAtTopOfKnee = float(AudioMath::gainFromDb(dbGainAtTopOfKnee));
    xy ret;
    ret.x = topOfKneeVin;
    ret.y = gainAtTopOfKnee;
    return ret;
}

CompCurves::xy CompCurves::getGainAtLeftInflection(const CompCurves::Recipe& r) {
    const float bottomOfKneeDb = -r.kneeWidth / 2;
    const float bottomOfKneeVin = float(AudioMath::gainFromDb(bottomOfKneeDb));
    xy ret;
    ret.x = bottomOfKneeVin;
    ret.y = 1;
    return ret;
}

CompCurves::LookupPtr CompCurves::makeCompGainLookup(const CompCurves::Recipe& r) {
    //assert(r.threshold > 0);
    CompCurves::LookupPtr ret = std::make_shared<NonUniformLookupTableParams<float>>();
    if (r.kneeWidth == 0) {
        auto lastPt = addLeftSideCurve(ret, r);
        addRightSideCurve(ret, r, lastPt);
    } else {
        // TODO: add middle
        auto lastPtLeft = addLeftSideCurve(ret, r);

        // todo return points
        addMiddleCurve(ret, r, lastPtLeft);

        auto lastPtRight = getGainAtRightInflection(r);
        addRightSideCurve(ret, r, lastPtRight);
    }
    NonUniformLookupTable<float>::finalize(*ret);
    return ret;
}

CompCurves::xy CompCurves::addLeftSideCurve(LookupPtr ptr, const Recipe& r) {
    const float bottomOfKneeDb = -r.kneeWidth / 2;
    const float bottomOfKneeVin = float(AudioMath::gainFromDb(bottomOfKneeDb));

    NonUniformLookupTable<float>::addPoint(*ptr, 0, 1);
    NonUniformLookupTable<float>::addPoint(*ptr, bottomOfKneeVin, 1);

    //printf("left side at 0,1 and %f,1\n", bottomOfKneeVin);

    return xy(bottomOfKneeVin, bottomOfKneeVin);
}

void CompCurves::addMiddleCurve(LookupPtr table, const Recipe& r, CompCurves::xy init) {
    assert(r.ratio > 1);
    // where we start curve
    const double x0Db = -r.kneeWidth / 2;
    // const double y0Db = AudioMath::db(init.y);

    const double x1Db = r.kneeWidth / 2;

    for (double xDb = x0Db; xDb <= x1Db; xDb += 1) {
        const double squareTerm = (xDb + r.kneeWidth / 2);
        const double rTerm = (1.0 / r.ratio) - 1;

        const double yDb = xDb + rTerm * squareTerm * squareTerm / (2 * r.kneeWidth);
        //const double yDb = xDb + (-1 + (1 / r.ratio)) * squareTerm * squareTerm / (2 * r.kneeWidth);

        const float x = float(AudioMath::gainFromDb(xDb));
        const float yV = float(AudioMath::gainFromDb(yDb));
        const float gain = yV / x;

        // printf("in middel x=%f ydb=%f v-g=%f, %f\n", xDb, yDb, x, gain);
        NonUniformLookupTable<float>::addPoint(*table, x, gain);
    }

    //assert(false);
}

void CompCurves::addRightSideCurve(LookupPtr table, const Recipe& r, CompCurves::xy init) {
    assert(r.kneeWidth < 20);  // code below will fail it so
    // start of left curve
    const double x0Db = r.kneeWidth / 2;
    const double dbSlope = 1.0 / r.ratio;

    const double x1Db = AudioMath::db(10);   // let's plot out to +0 db
    const double x2Db = AudioMath::db(100);  // no, let's have a 40 db range!

    // printf("x0 db = %f, x1Db = %f, x2 = %f\n", x0Db, x1Db, x2Db );
    double incrementDb = 1;
    for (double xDb = x0Db; xDb <= x2Db; xDb += incrementDb) {
        const double yDb = dbSlope * xDb;

        const float x = float(AudioMath::gainFromDb(xDb));
        const float gain = float(AudioMath::gainFromDb(yDb)) / x;
        // printf("new R: another right point db=%f,%f v-g=%f,%f\n", xDb, yDb, x, gain);
        NonUniformLookupTable<float>::addPoint(*table, x, gain);

        // for tons of comp, we can be a little less precise.
        incrementDb = xDb > x1Db ? 3 : 1;
    }
}

std::function<double(double)> CompCurves::_getContinuousCurve(const CompCurves::Recipe& r, bool bUseSplines) {
    //  assert(!bUseSplines);
    std::shared_ptr<NonUniformLookupTableParams<double>> splineLookup;
    if (bUseSplines) {
        splineLookup = makeSplineMiddle(r);
    }
    return [r, bUseSplines, splineLookup](double x) {
        const double bottomOfKneeDb = -r.kneeWidth / 2;
        const double bottomOfKneeVin = float(AudioMath::gainFromDb(bottomOfKneeDb));
        const double topOfKneeDb = r.kneeWidth / 2;
        const double topOfKneeVin = float(AudioMath::gainFromDb(topOfKneeDb));

        if (x < bottomOfKneeVin) {
            // SQINFO("left side");
            return 1.0;  // constant gain of 1 below thresh
        } else if (x < topOfKneeVin) {
            // const double x0Db = bottomOfKneeDb;
            if (!bUseSplines) {
                const double xdb = AudioMath::db(x);
                const double squareTerm = (xdb + r.kneeWidth / 2);
                const double rTerm = (1.0 / r.ratio) - 1;
                const double yDb = xdb + rTerm * squareTerm * squareTerm / (2 * r.kneeWidth);
                const double x2 = x;

                const double yV = AudioMath::gainFromDb(yDb);
                const double gain = yV / x2;
                return gain;
            } else {
                assert(splineLookup);
                return NonUniformLookupTable<double>::lookup(*splineLookup, x);
                // return double(CompCurves::lookup(splineLookup, float(x)));
            }
        } else {
            const double xdb = AudioMath::db(x);
            const double dbSlope = 1.0 / r.ratio;

            //   const double x1Db = AudioMath::db(10);   // let's plot out to +0 db
            //  const double x2Db = AudioMath::db(100);  // no, let's have a 40 db range!

            const double yDb = dbSlope * xdb;

            const double xTest = AudioMath::gainFromDb(xdb);
            assertClose(x, xTest, .000001);
            const double gain = AudioMath::gainFromDb(yDb) / x;

            return gain;
        }
        return 0.0;
    };
}

CompCurves::CompCurveLookupPtr CompCurves::makeCompGainLookup2(const Recipe& r) {
    return makeCompGainLookupEither(r, false);
}

CompCurves::CompCurveLookupPtr CompCurves::makeCompGainLookup3(const Recipe& r) {
    return makeCompGainLookupEither(r, true);
}

// at 10, distortion jumps at the changes
// 100 it does a little bit? but 1000 is too high...
const int tableSize = 100;
CompCurves::CompCurveLookupPtr CompCurves::makeCompGainLookupEither(const Recipe& r, bool bUseSpline) {
    //   assert(!bUseSpline);
    CompCurveLookupPtr ret = std::make_shared<CompCurveLookup>();

    const float bottomOfKneeDb = -r.kneeWidth / 2;
    ret->bottomOfKneeVin = float(AudioMath::gainFromDb(bottomOfKneeDb));

    // const float topOfKneeDb = r.kneeWidth / 2;
    //  const float topOfKneeVin = float(AudioMath::gainFromDb(topOfKneeDb));
    ret->dividingLine = 2;

    auto func = _getContinuousCurve(r, bUseSpline);

    LookupTable<float>::init(ret->lowRange, tableSize, ret->bottomOfKneeVin, ret->dividingLine, func);
    LookupTable<float>::init(ret->highRange, tableSize, ret->dividingLine, 100, func);
    return ret;
}

float CompCurves::CompCurveLookup::lookup(float x) const {
    if (x <= bottomOfKneeVin) {
        return 1;
    }
    if (x < dividingLine) {
        return LookupTable<float>::lookup(lowRange, x, false);
    }
    return LookupTable<float>::lookup(highRange, x, true);
}

std::function<float(float)> CompCurves::getLambda(const Recipe& r, Type t) {
    std::function<float(float)> ret;

    switch (t) {
        case Type::ClassicNU: {
            CompCurves::LookupPtr classicNUPtr = CompCurves::makeCompGainLookup(r);
            ret = [classicNUPtr](float x) {
                return CompCurves::lookup(classicNUPtr, x);
            };
        } break;
        case Type::ClassicLin: {
            CompCurves::CompCurveLookupPtr classicLinPtr = CompCurves::makeCompGainLookup2(r);
            ret = [classicLinPtr](float x) {
                return classicLinPtr->lookup(x);
            };
        } break;
        default:
            assert(false);
    }
    return ret;
}

std::shared_ptr<NonUniformLookupTableParams<double>>
CompCurves::makeSplineMiddle(const Recipe& r) {
    std::shared_ptr<NonUniformLookupTableParams<double>> firstTableParam = std::make_shared<NonUniformLookupTableParams<double>>();
    {
        // Make a hermite spline from the Recipe
        auto spline = HermiteSpline::make(int(std::round(r.ratio)), int(std::round(r.kneeWidth)));
        if (!spline) {
            return nullptr;
        }

        // First make a non-uniform for mapping that hermite with linear axis (wrong)
        //std::shared_ptr<NonUniformLookupTableParams<double>> firstTableParam = std::make_shared<NonUniformLookupTableParams<double>>();
        int iNum = 1024;
        for (int i = 0; i < 1024; ++i) {
            double t = double(i) / double(iNum);
            auto pt = spline->renderPoint(t);
            NonUniformLookupTable<double>::addPoint(*firstTableParam, pt.first, pt.second);
        }
        NonUniformLookupTable<double>::finalize(*firstTableParam);
    }

    // Render the warped spline into a non-uniform lookup table so we can do cartesian mapping (later)
    // (for now we are only doing non-uniform. Until it works right).
    std::shared_ptr<NonUniformLookupTableParams<double>> params = std::make_shared<NonUniformLookupTableParams<double>>();
    int iNum2 = 1024;
    //int iNum2 = 12;
    for (int i = 0; i <= iNum2; ++i) {
        // assert false - this is all old ATM
        //
        // map i continusously into .5 ... 2 (the knee input level in volts)
        double vInVolts = double(i) / double(iNum2);  // 0..1
        vInVolts *= 1.5;                              // 0..1.5
        vInVolts += .5;                               // .5..2. That's that the curve was designed for

        double vInDb = AudioMath::db(vInVolts);  // -6 ... 6
      

        // our spline lookup give the gain in DB for an input in DB, -6..+6 in
        double yGainDb = NonUniformLookupTable<double>::lookup(*firstTableParam, vInDb);
        double yGainVolts = AudioMath::gainFromDb(yGainDb);

        double yOutDebug = yGainVolts * vInVolts;
      
        NonUniformLookupTable<double>::addPoint(*params, vInVolts, yGainVolts);

       // SQINFO("i=%d  vInVolts=%f vInDb =%f", i, vInVolts, vInDb);
       // SQINFO("yGainDb=%f yg=%f debug=%f\n", yGainDb, yGainVolts, yOutDebug);
    }

    //assert(false);
    NonUniformLookupTable<double>::finalize(*params);
    return params;
}

#if 0  // third try (final with old alg)
std::shared_ptr<NonUniformLookupTableParams<double>>
CompCurves::makeSplineMiddle(const Recipe& r) {
    std::shared_ptr<NonUniformLookupTableParams<double>> firstTableParam = std::make_shared<NonUniformLookupTableParams<double>>();
    {
        // Make a hermite spline from the Recipe
        auto spline = HermiteSpline::make(int(std::round(r.ratio)), int(std::round(r.kneeWidth)));
        if (!spline) {
            return nullptr;
        }

        // First make a non-uniform for mapping that hermite with linear axis (wrong)
        //std::shared_ptr<NonUniformLookupTableParams<double>> firstTableParam = std::make_shared<NonUniformLookupTableParams<double>>();
        int iNum = 1024;
        for (int i = 0; i < 1024; ++i) {
            double t = double(i) / double(iNum);
            auto pt = spline->renderPoint(t);
            NonUniformLookupTable<double>::addPoint(*firstTableParam, pt.first, pt.second);
        }
        NonUniformLookupTable<double>::finalize(*firstTableParam);
    }

    // next, make a new table using db warping on the x axit

    // Render the spline into a non-uniform lookup table so we can do cartesian mapping.
    std::shared_ptr<NonUniformLookupTableParams<double>> params = std::make_shared<NonUniformLookupTableParams<double>>();
    int iNum2 = 1024;
    //int iNum2 = 12;
    for (int i = 0; i <= iNum2; ++i) {
        // Put x into DB, bu scale it back to fit in .5 .. 2
        double x = double(i) / double(iNum2);  // 0..1
        x += 1;                                // 1..2

        double xDb = AudioMath::db(x);  // 0 ... 6
        xDb /= 6;                       // now 0..1
        xDb *= 1.5;                     // now 0..1.5
        xDb += .5;                      // now .5..2

        // derive an x that goes from .5 to 2, linearly
        double xLin_ = double(i) / double(iNum2);  // 0..1
        xLin_ *= 1.5;                               // 0.. 1.5
        xLin_ += .5;                                // .5 .. 2

        // now: final table x = dDb  y = f(xLin)
        double y = NonUniformLookupTable<double>::lookup(*firstTableParam, xDb);
        double gain = y / xDb;

        // should we add gain or y value??? I don't know
        NonUniformLookupTable<double>::addPoint(*params, xDb, gain);

        //SQINFO("i=%d  xLin=%f xDb =%f y=%f gain=%f", i, xLin_, xDb, y, gain);
    }

    //assert(false);
    NonUniformLookupTable<double>::finalize(*params);

    return params;
}
#endif

#if 0  // second try
std::shared_ptr<NonUniformLookupTableParams<double>>
CompCurves::makeSplineMiddle(const Recipe& r) {
    std::shared_ptr<NonUniformLookupTableParams<double>> firstTableParam = std::make_shared<NonUniformLookupTableParams<double>>();
    {
        // Make a hermite spline from the Recipe
        auto spline = HermiteSpline::make(int(std::round(r.ratio)), int(std::round(r.kneeWidth)));
        if (!spline) {
            return nullptr;
        }

        // First make a non-uniform for mapping that hermite with linear axis (wrong)
        //std::shared_ptr<NonUniformLookupTableParams<double>> firstTableParam = std::make_shared<NonUniformLookupTableParams<double>>();
        int iNum = 1024;
        for (int i = 0; i < 1024; ++i) {
            double t = double(i) / double(iNum);
            auto pt = spline->renderPoint(t);
            NonUniformLookupTable<double>::addPoint(*firstTableParam, pt.first, pt.second);
        }
        NonUniformLookupTable<double>::finalize(*firstTableParam);
    }

    // next, make a new table using db warping on the x axit

    // Render the spline into a non-uniform lookup table so we can do cartesian mapping.
    std::shared_ptr<NonUniformLookupTableParams<double>> params = std::make_shared<NonUniformLookupTableParams<double>>();
    // int iNum = 1024;
    int iNum2 = 12;
    for (int i = 0; i <= iNum2; ++i) {
        // Put x into DB, bu scale it back to fit in .5 .. 2
        double x = double(i) / double(iNum2);  // 0..1
        x += 1;                               // 1..2

        double xDb = AudioMath::db(x);  // 0 ... 6
        xDb /= 6;                       // now 0..1
        xDb *= 1.5;                     // now 0..1.5
        xDb += .5;                      // now .5..2

        // derive an x that goes from .5 to 2, linearly
        double xLin = double(i) / double(iNum2);  // 0..1
        xLin *= 1.5;                             // 0.. 1.5
        xLin += .5;                              // .5 .. 2

        // now: final table x = dDb  y = f(xLin)
        double y = NonUniformLookupTable<double>::lookup(*firstTableParam, xLin);
        double gain = y / xLin;
        NonUniformLookupTable<double>::addPoint(*params, xDb, y);

        SQINFO("i=%d  xLin=%f xDb =%f y=%f gain=%f", i, xLin, xDb, y, gain);

#if 0
        // x = .5...2 = - 6db to +6 db
        double xInputVoltage = pt.first;
        double xInputDb = AudioMath::db(xInputVoltage);

        // now rescale input db to be in the range .5 .. 2
       
        double xWarpedInputDb = x1;

        double outputDb = AudioMath::db(pt.second);
        double gainDb = outputDb - xInputDb;
        double gain = AudioMath::gainFromDb(gainDb);


        SQINFO("i=%d t=%f x=%f xInputDb=%f warpedInputDb=%f", i, t, xInputVoltage, xInputDb, xWarpedInputDb);
        SQINFO("output=%f outputDb=%f gainDb=%f gain=%f", pt.second, outputDb, gainDb, gain);
#endif
        // NonUniformLookupTable<double>::addPoint(*params, pt.first, pt.second);
    }

    assert(false);
    NonUniformLookupTable<double>::finalize(*params);

    return params;
}
#endif

#if 0  // first try
std::shared_ptr<NonUniformLookupTableParams<double>>
CompCurves::makeSplineMiddle(const Recipe& r) {
    auto spline = HermiteSpline::make(int(std::round(r.ratio)), int(std::round(r.kneeWidth)));
    if (!spline) {
        return nullptr;
    }

    // First make a non-uniform for
    std::shared_ptr <NonUniformLookupTableParams<double>> firstTableParam = std::make_shared<NonUniformLookupTableParams<double>>();
    int iNum = 1024;
    for (int i = 0; i < 1024; ++i) {
        double t = double(i) / double(iNum);
        auto pt = spline->renderPoint(t);
        NonUniformLookupTable<double>::addPoint(*firstTableParam, pt.first, pt.second);
    }
    NonUniformLookupTable<double>::finalize(*firstTableParam);

    return firstTableParam;
}
#endif
