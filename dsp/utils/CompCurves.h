#pragma once

#include <functional>
#include <vector>



/*

more general spline ideas.
how does asym shaper work?
    it knows how to draw one spline.
    it renders the spline into a non uniform lookup.
    then a uniform lookup is generated from it.

Let's do a new one:
    abstract spline  can generate x, y from t = {0..1}
    let's use the standard non-uniform

    class BaseSpline
    call HermiteSpline

*/
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
private:
    static std::vector<xy> makeLeftGainTableSection(const Recipe& r);
    static std::vector<xy> makeMiddleGainTableSection(const Recipe& r);
    static std::vector<xy> makeRightGainTableSection(const Recipe& r);

    static std::vector<xy> makeCrudeCompGainTableNoKnee(const Recipe&);
    static std::vector<xy> makeCrudeCompGainTableKnee(const Recipe&);
};