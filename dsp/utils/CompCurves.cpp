

#include "CompCurves.h"

#include "asserts.h"
#include <functional>

CompCurves::LookupPtr CompCurves::makeCompGainLookup(const CompCurves::Recipe& r)
{
    assert(r.threshold > 0);
    CompCurves::LookupPtr ret = std::make_shared<NonUniformLookupTableParams<float>>();
    if (r.kneeWidth == 0) {
        auto lastPt = addLeftSideCurve(ret, r);
        addRightSideCurve(ret, r, lastPt);
    } else {
        assert(false);
    }
    NonUniformLookupTable<float>::finalize(*ret);
    return ret;
}

CompCurves::xy CompCurves::addLeftSideCurve(LookupPtr ptr, const Recipe& r)
{
    printf("add left side\n");
    NonUniformLookupTable<float>::addPoint(*ptr, 0, 1);
    NonUniformLookupTable<float>::addPoint(*ptr, r.threshold, 1);
    assert(r.kneeWidth == 0);   // if non-zero, then the above is wrong - it's no linear gain up to thresh

    printf("added 0,1 and %f,1\n", r.threshold);
    return xy(r.threshold, r.threshold);
}

void CompCurves::addRightSideCurve(LookupPtr table, const Recipe& r, CompCurves::xy init)
{
    printf("add right side\n");
    const double x0Db = AudioMath::db(init.x);
    const double y0Db = AudioMath::db(init.y);

    // let's calc straight line db from thresh to 10, db(10)
   // CompCurves::xy secondPoint
    assert(r.threshold < 10);
   // double gainAt10 = 
    const double x1Db = AudioMath::db(10);      // line ends at x = gain of 10

    // this isn't quite right. when it was r.ratio it seemed to increase ok.
    const double dbSlope = 1.0 / r.ratio;

    assert(x1Db > x0Db);
   // assert(dbSlope = 1);

    // TODO: what range do we need in this table?
    const double finalX = (x1Db);

    for (double xDb = x0Db; xDb <= finalX; xDb += 1) {
        double yDb = y0Db + dbSlope * (xDb - x0Db);

        const float dbChange = float(yDb - xDb);
        const float x = float(AudioMath::gainFromDb(xDb));
       // const float yGain = float(AudioMath::gainFromDb(yDb));
        float gain = float(AudioMath::gainFromDb(dbChange));

        printf("another point db=%f,%f g=%f,%f\n", xDb, yDb, x, gain);
        NonUniformLookupTable<float>::addPoint(*table, x, gain);
    }
}
