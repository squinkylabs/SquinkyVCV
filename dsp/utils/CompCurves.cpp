

#include "CompCurves.h"

#include <functional>

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

std::function<double(double)> CompCurves::_getContinuousCurve(const CompCurves::Recipe& r) {
    // assert(false);
    return [r](double x) {
        const double bottomOfKneeDb = -r.kneeWidth / 2;
        const double bottomOfKneeVin = float(AudioMath::gainFromDb(bottomOfKneeDb));
        const double topOfKneeDb = r.kneeWidth / 2;
        const double topOfKneeVin = float(AudioMath::gainFromDb(topOfKneeDb));

        if (x < bottomOfKneeVin) {
            // SQINFO("left side");
            return 1.0;  // constant gain of 1 below thresh
        } else if (x < topOfKneeVin) {
            const double x0Db = bottomOfKneeDb;
            const double xdb = AudioMath::db(x);
            const double squareTerm = (xdb + r.kneeWidth / 2);
            const double rTerm = (1.0 / r.ratio) - 1;
            const double yDb = xdb + rTerm * squareTerm * squareTerm / (2 * r.kneeWidth);
            const double x2 = x;

            const double yV = AudioMath::gainFromDb(yDb);
            const double gain = yV / x2;
            return gain;
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

/*
    class CompCurveLookup {
    public:
        float lookup(float x) {
            return 0;
        }
    private:
        LookupTableParams<float> lowRange;
        LookupTableParams<float> highange;
        float dividingLine = 0;                 // where we switch tables
        float bottomOfKneeVin = 0;              // below here gain is one
    };

template<typename T>
inline void LookupTable<T>::init(LookupTableParams<T>& params,
    int bins, T x0In, T x1In, std::function<double(double)> f)
{
*/
CompCurves::CompCurveLookupPtr CompCurves::makeCompGainLookup2(const Recipe& r) {
    CompCurveLookupPtr ret = std::make_shared<CompCurveLookup>();

    const float bottomOfKneeDb = -r.kneeWidth / 2;
    ret->bottomOfKneeVin = float(AudioMath::gainFromDb(bottomOfKneeDb));
    
    const float topOfKneeDb = r.kneeWidth / 2;
    const float topOfKneeVin = float(AudioMath::gainFromDb(topOfKneeDb));
    ret->dividingLine = 2;

    auto func = _getContinuousCurve(r);

    LookupTable<float>::init(ret->lowRange, 100, ret->bottomOfKneeVin, ret->dividingLine, func);
    LookupTable<float>::init(ret->highRange, 100, ret->dividingLine, 100, func);
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
