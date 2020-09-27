

#include "CompCurves.h"

#include "asserts.h"
#include <functional>



static std::function<float(float)> continuousGainFunction(const CompCurves::Recipe& r)
{
    return [](float x) {
        return x;
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
                points.pop_back();
                points.push_back(CompCurves::xy(x, y));
            }
            else {
                points.push_back(CompCurves::xy(x, y));
            }
        }
        x += deltaX;
        if (x > r.maxX) {
            done = true;
        }
    }
    return points;
   
}