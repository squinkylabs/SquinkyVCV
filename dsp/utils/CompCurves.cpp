

#include "CompCurves.h"

#include "asserts.h"
#include <functional>



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
std::vector<CompCurves::xy> CompCurves::makeCrudeCompGainTable(const Recipe& r)
{
    std::vector<CompCurves::xy> points;
    auto func = continuousGainFunction(r);
    bool done = false;
    float x = r.minX;
    const float deltaX = r.yError;
    //const float deltaX = 1;     // just for test
    bool lastPointSkipped = false;
    CompCurves::xy lastPoint;


    while (!done) {
        float y = func(x);
        if (points.size() < 2) {
            points.push_back(CompCurves::xy(x, y));
        }
        else {
            CompCurves::xy firstPoint = points.at(points.size() - 2);
            CompCurves::xy secondPoint = points.back();
            CompCurves::xy currentPoint(x, y);

            // calculate a line between first point and second point
            // f = ax + b
            const float slope = (secondPoint.y - firstPoint.y) / (secondPoint.x - firstPoint.x);
            const float offset = firstPoint.y;

            {
                // some match checking
                float firstY = offset;
                assert(firstY == firstPoint.y);

                float secondY = slope * (secondPoint.x - firstPoint.x) + offset;
                assertClose(secondY, secondPoint.y, (r.yError / 2));
            }

            float interpolatedCurrentPointY = slope * (currentPoint.x - firstPoint.x) + offset;
            float errorY = std::abs(interpolatedCurrentPointY - currentPoint.y);
            if (errorY < (r.yError / 2)) {
                // the three points are on a straight line. remove second and replace with current
              //  points.pop_back();
              //  points.push_back(CompCurves::xy(x, y));
                lastPointSkipped = true;
            }
            else {
                //assert(false);
                lastPointSkipped = false;
                points.push_back(CompCurves::xy(x, y));
            }
            lastPoint = currentPoint;
        }
        x += deltaX;
        if (x > r.maxX) {
            done = true;
        }

    }
    return points;

}

#if 0
std::vector<CompCurves::xy> CompCurves::makeCrudeCompGainTable(const Recipe& r)
{
    std::vector<CompCurves::xy> points;
    auto func = continuousGainFunction(r);
    bool done = false;
    float x = r.minX;
    const float deltaX = r.yError;
    bool lastPointSkipped = false;
    CompCurves::xy lastPoint;


    while (!done) {
        float y = func(x);
        if (points.size() < 2) {
            points.push_back( CompCurves::xy(x, y));
        }
        else {
            CompCurves::xy firstPoint = points.at(points.size() - 2);
            CompCurves::xy secondPoint = points.back();
            CompCurves::xy currentPoint(x, y);

            // calculate a line between first point and current points
            // f = ax + b
            const float slope = (currentPoint.y - firstPoint.y) / (currentPoint.x - firstPoint.x);
            const float offset = firstPoint.y;

            {
                // some match checking
                float firstY = offset;
                assert(firstY == firstPoint.y);

                float currentY = slope * (currentPoint.x - firstPoint.x) + offset;
                assertClose(currentY, currentPoint.y, (r.yError / 2));
            }

            float interpolatedSecondPointY = slope * (secondPoint.x - firstPoint.x) + offset;
            float errorY = std::abs(interpolatedSecondPointY - secondPoint.y);
            if (errorY < (r.yError / 2)) {
                // the three points are on a straight line. remove second and replace with current
              //  points.pop_back();
              //  points.push_back(CompCurves::xy(x, y));
                lastPointSkipped = true;
            }
            else {
                assert(false);
                lastPointSkipped = false;
                points.push_back(CompCurves::xy(x, y));
            }
            lastPoint = currentPoint;
        }
        x += deltaX;
        if (x > r.maxX) {
            done = true;
        }
       
    }
    return points;
   
}
#endif