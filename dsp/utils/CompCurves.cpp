

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
    NonUniformLookupTable<float>::addPoint(*ptr, 0, 0);
    NonUniformLookupTable<float>::addPoint(*ptr, r.threshold, r.threshold);
    assert(r.kneeWidth == 0);   // if non-zero, then the above is wrong - it's no linear gain up to thresh
    return xy(r.threshold, r.threshold);
}

void CompCurves::addRightSideCurve(LookupPtr table, const Recipe& r, CompCurves::xy init)
{
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

      //  printf("another point db=%f,%f g=%f,%f\n", xDb, yDb, x, gain);
        NonUniformLookupTable<float>::addPoint(*table, x, gain);
    }

    
}


//////////////////////// DPRECATED STUFF
std::function<float(float)> CompCurves::continuousGainFunction(const CompCurves::Recipe& r)
{
    assert(r.kneeWidth == 0);
    assert(r.minX < r.threshold);
    assert(r.maxX > r.threshold);
    assert(r.ratio > 0);

    // only works for no knee
    // compute line segment for first half
 //   const float slope = (currentPoint.y - firstPoint.y) / (currentPoint.x - firstPoint.x);
 //   const float offset = firstPoint.y;

    const float threshold = r.threshold;
    const float slope1 = 1;
    const float offset1 = 0;

    const float slope2 = 1 / r.ratio;
    const float uncompY2 = slope2 * threshold;
    const float desiredY2 = threshold;

    const float offset2 = desiredY2 - uncompY2;

    return [threshold, slope1, slope2, offset2](float x) {
        if (x < threshold) {
            return x * slope1;
        }
        else {
            return x * slope2 + offset2;
        }
    };
}


/*
float ratio=1;
float kneeWidth=0;
float yError=.1f;
float minX=-50;
float maxX=50;
float threshold=1;
*/

std::vector<CompCurves::xy> CompCurves::makeMiddleGainTableSection(const Recipe& r)
{
    std::vector <CompCurves::xy> points;
    return points;
}

std::vector<CompCurves::xy> CompCurves::makeLeftGainTableSection(const Recipe& r)
{
    std::vector <CompCurves::xy> points;
    points.push_back(CompCurves::xy(r.minX, r.minX));
    points.push_back(CompCurves::xy(r.threshold, r.threshold));
    return points;
}

std::vector<CompCurves::xy> CompCurves::makeRightGainTableSection(const Recipe& r)
{
    std::vector <CompCurves::xy> points;

    const float slope2 = 1 / r.ratio;
    const float uncompY2 = slope2 * r.threshold;
    const float desiredY2 = r.threshold;

    const float offset2 = desiredY2 - uncompY2;

    points.push_back(CompCurves::xy(r.threshold, r.threshold));
    points.push_back(CompCurves::xy(r.maxX, r.maxX * slope2 + offset2));
    return points;
}

std::vector<CompCurves::xy> CompCurves::makeCrudeCompGainTableNoKnee(const Recipe& r)
{
    assert(r.kneeWidth == 0);
    auto leftLine =  makeLeftGainTableSection(r);
    auto rightLine = makeRightGainTableSection(r);

    auto rightBegin = rightLine.begin();

    // we expect threshold to be duplicated/
    assertClose(leftLine.back().x, rightLine.front().x, .0001);
    rightBegin++;


    leftLine.insert(leftLine.end(), rightBegin, rightLine.end());
    return leftLine;
}

std::vector<CompCurves::xy> CompCurves::makeCrudeCompGainTableKnee(const Recipe& r)
{
    //std::vector <CompCurves::xy> points;
    assert(r.kneeWidth > 0);
    auto leftLine = makeLeftGainTableSection(r);
    auto rightLine = makeRightGainTableSection(r);
    auto middleCurve = makeMiddleGainTableSection(r);
   
    assert(!middleCurve.empty());
    auto middleBegin = middleCurve.begin();
    assertClose(leftLine.back().x, middleCurve.front().x, .0001);
    middleBegin++;
    leftLine.insert(leftLine.end(), middleBegin, middleCurve.end());

    auto rightBegin = rightLine.begin();
    assertClose(leftLine.back().x, rightLine.front().x, .0001);
    rightBegin++;
    leftLine.insert(leftLine.end(), rightBegin, rightLine.end());

    return leftLine;
}

std::vector<CompCurves::xy> CompCurves::makeCrudeCompGainTable(const Recipe& r)
{
    return (r.kneeWidth == 0) ?
        makeCrudeCompGainTableNoKnee(r) :
        makeCrudeCompGainTableKnee(r);
}
