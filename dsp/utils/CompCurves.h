#pragma once

#include "NonUniformLookupTable.h"
#include <functional>
#include <memory>
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
    CompCurves() = delete;

    /**
     * All the parameters to define a gain curve
     * Note that ratio below knee is always 1:1
     */
    class Recipe
    {
    public:
        /**
         * 2 means 2:1 compression ratio.
         * ratio is in decibels.
         */
        float ratio=1;
        float kneeWidth=0;
        float yError=.1f;

      

        /**
         * Threshold is in volts
         */
        float threshold=1;

        /**
         * Deprecated
         * minimum and maximum signal levels we care about.
         * TBD: we care about levels all the way down to zer
         */
        float minX=-50;
        float maxX=50;

    };

     /**
     * returns a lookup table that define a gain curve.
     * gain = Vout / Vin. Not decibels, voltage ratio.
     */
    using LookupPtr = std::shared_ptr<NonUniformLookupTableParams<float>>; 
    using LookupPtrConst = std::shared_ptr<const NonUniformLookupTableParams<float>>; 
    static LookupPtr makeCompGainLookup(const Recipe&);

    static float lookup(LookupPtrConst table, float x) {
        return NonUniformLookupTable<float>::lookup(*table, x);
    }

    /**
     * returns a series of points that define a gain curve.
     * removed interior points that are on a straight line.
     * DEPRECATED
     */

    class xy
    {
    public:
        xy(float a, float b) : x(a), y(b) {}
        xy() = default;
        float x = 0;
        float y = 0;

    };
    static std::vector<xy> makeCrudeCompGainTable(const Recipe&);
    static std::function<float(float)> continuousGainFunction(const CompCurves::Recipe& r);



private:
    static xy addLeftSideCurve(LookupPtr, const Recipe& r);
    static void addRightSideCurve(LookupPtr, const Recipe& r, xy lastPt);

    /**
     * Deprecated
     */
    static std::vector<xy> makeLeftGainTableSection(const Recipe& r);
    static std::vector<xy> makeMiddleGainTableSection(const Recipe& r);
    static std::vector<xy> makeRightGainTableSection(const Recipe& r);
    static std::vector<xy> makeCrudeCompGainTableNoKnee(const Recipe&);
    static std::vector<xy> makeCrudeCompGainTableKnee(const Recipe&);
};