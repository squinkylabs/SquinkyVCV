#pragma once

#include <functional>
#include <vector>

class CompCurves
{
public:
    class xy
    {
    public:
        xy(float a, float b) : x(a), y(b) {}
        xy() = default;
        float x=0;
        float y=0;
    };

    class Recipe
    {
    public:
        /**
         * 2 means 2:1 compression ratio.
         */
        float ratio=1;
        float kneeWidth=0;
        float yError=.1f;
        float minX=-50;
        float maxX=50;
        float threshold=1;
    };

    /**
     * returns a series of points that define a gain curve.
     * removed interior points that are on a straight line
     */
    static std::vector<xy> makeCrudeCompGainTable(const Recipe&);

    static std::function<float(float)> continuousGainFunction(const CompCurves::Recipe& r);
};