#pragma once

#include "NonUniformLookupTable.h"
#include <functional>
#include <memory>
#include <vector>



/*
knee plan, phase 2

Try plotting a hard knee in excel or something. what does it look like.
Determine points and slopes for end of knee.
Try implementing - does it pass unit tests?
Beef up unit tests (decreasing slope)
Plot soft knee in excel.

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

};