

#include "CompCurves.h"

#include "asserts.h"
#include <functional>


CompCurves::xy CompCurves::getGainAtRightInflection(const CompCurves::Recipe& r)
{
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

CompCurves::xy  CompCurves::getGainAtLeftInflection(const CompCurves::Recipe& r)
{

    const float bottomOfKneeDb = -r.kneeWidth / 2;
    const float bottomOfKneeVin = float(AudioMath::gainFromDb(bottomOfKneeDb));
    xy ret;
    ret.x = bottomOfKneeVin;
    ret.y = 1;
    return ret;
}

CompCurves::LookupPtr CompCurves::makeCompGainLookup(const CompCurves::Recipe& r)
{
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

CompCurves::xy CompCurves::addLeftSideCurve(LookupPtr ptr, const Recipe& r)
{
    const float bottomOfKneeDb = - r.kneeWidth / 2;
    const float bottomOfKneeVin = float(AudioMath::gainFromDb(bottomOfKneeDb));

    NonUniformLookupTable<float>::addPoint(*ptr, 0, 1);
    NonUniformLookupTable<float>::addPoint(*ptr, bottomOfKneeVin, 1);

    //printf("left side at 0,1 and %f,1\n", bottomOfKneeVin);

    return xy(bottomOfKneeVin, bottomOfKneeVin);
}


void CompCurves::addMiddleCurve(LookupPtr table, const Recipe& r, CompCurves::xy init)
{
    assert(r.ratio > 1);
    // where we start curve
    const double x0Db = - r.kneeWidth / 2;
    // const double y0Db = AudioMath::db(init.y);

    const double x1Db = r.kneeWidth / 2;

    for (double xDb = x0Db; xDb <= x1Db; xDb += 1) {
        const double squareTerm = (xDb + r.kneeWidth /2);
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

void CompCurves::addRightSideCurve(LookupPtr table, const Recipe& r, CompCurves::xy init)
{
     assert(r.kneeWidth < 20);       // code below will fail it so
    // start of left curve
    const double x0Db = r.kneeWidth / 2;
    const double dbSlope = 1.0 / r.ratio;

    const double x1Db = AudioMath::db(10);      // let's plot out to +0 db
    const double x2Db = AudioMath::db(100);      // no, let's have a 40 db range!

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


// original
#if 0
void CompCurves::addRightSideCurve(LookupPtr table, const Recipe& r, CompCurves::xy init)
{
  
  //  printf("add right side\n");
    const double x0Db = AudioMath::db(init.x);
    const double y0Db = AudioMath::db(init.y);

    assert(r.kneeWidth < 20);       // code below will fail it so

    // let's calc straight line db from thresh to 10, db(10)
   // double gainAt10 = 
    const double x1Db = AudioMath::db(10);      // line ends at x = gain of 10


    const double dbSlope = 1.0 / r.ratio;

    assert(x1Db > x0Db);
    const double finalX = (x1Db);

    for (double xDb = x0Db; xDb <= finalX; xDb += 1) {
        double yDb = y0Db + dbSlope * (xDb - x0Db);

        const float dbChange = float(yDb - xDb);
        const float x = float(AudioMath::gainFromDb(xDb));
       // const float yGain = float(AudioMath::gainFromDb(yDb));
        float gain = float(AudioMath::gainFromDb(dbChange));

        printf("orig: another right point db=%f,%f v-g=%f,%f\n", xDb, yDb, x, gain);
        //NonUniformLookupTable<float>::addPoint(*table, x, gain);
    }

   
}
#endif
