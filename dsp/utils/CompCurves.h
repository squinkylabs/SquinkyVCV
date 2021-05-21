#pragma once

#include "LookupTable.h"
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

    #define normalizedThreshold 1.f

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

        // Total knee width, in decibels
        float kneeWidth=0;
    };

    class CompCurveLookup {
    public:
        friend class CompCurves;
        float lookup(float) const;
    private:
        LookupTableParams<float> lowRange;
        LookupTableParams<float> highRange;
        float dividingLine = 0;                 // where we switch tables
        float bottomOfKneeVin = 0;              // below here gain is one
    };

    using CompCurveLookupPtr = std::shared_ptr<CompCurveLookup>;

    static CompCurveLookupPtr makeCompGainLookup2(const Recipe&);

     /**
      * These for the non-uniform lookups
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
    static xy getGainAtRightInflection(const CompCurves::Recipe& r);
    static xy getGainAtLeftInflection(const CompCurves::Recipe& r);


    static std::function<double(double)> _getContinuousCurve(const CompCurves::Recipe& r, bool useSpline);
private:
    static xy addLeftSideCurve(LookupPtr, const Recipe& r);
    static void addRightSideCurve(LookupPtr, const Recipe& r, xy lastPt);
    static void addMiddleCurve(LookupPtr, const Recipe& r, xy lastPt);

   // static LookupPtr makeSplineMiddle(const Recipe&);
  
   
    //static CompCurvePtr makeCompCurveMiddle(const Recipe& r);
    static std::shared_ptr<NonUniformLookupTableParams<double>> makeSplineMiddle(const Recipe&);

};