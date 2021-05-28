#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "LookupTable.h"
#include "NonUniformLookupTable.h"

/*
knee plan, phase 2

Try plotting a hard knee in excel or something. what does it look like.
Determine points and slopes for end of knee.
Try implementing - does it pass unit tests?
Beef up unit tests (decreasing slope)
Plot soft knee in excel.

*/

class CompCurves {
public:
    CompCurves() = delete;

#define normalizedThreshold 1.f

    /**
     * All the parameters to define a gain curve
     * Note that ratio below knee is always 1:1
     */
    class Recipe {
    public:
        Recipe(float r, float k) : ratio(r), kneeWidth(k) {}
        Recipe() = default;
        /**
         * 2 means 2:1 compression ratio.
         * ratio is in decibels.
         */
        float ratio = 1;

        // Total knee width, in decibels
        float kneeWidth = 0;
    };

    using T = float;
    class CompCurveLookup {
    public:
        friend class CompCurves;
        float lookup(float) const;
        void _dump() const;
    private:
        LookupTableParams<T> lowRange;
        LookupTableParams<T> highRange;
        float dividingLine = 0;     // where we switch tables
        float bottomOfKneeVin = 0;  // below here gain is one
    };

    using CompCurveLookupPtr = std::shared_ptr<CompCurveLookup>;

    /**
     * makes two linear lookups, aka "fast"
     * still uses the old parabolic knee
     */
    static CompCurveLookupPtr makeCompGainLookup2(const Recipe&);

    /**
     * makes two linear lookups, aka "fast"
     * uses the new spline knee
     */
    static CompCurveLookupPtr makeCompGainLookup3(const Recipe&);

    static CompCurveLookupPtr makeCompGainLookupEither(const Recipe&, bool bUseSpline);

    /**
      * These for the non-uniform lookups
      * returns a lookup table that define a gain curve.
      * gain = Vout / Vin. Not decibels, voltage ratio.
      */
    using LookupPtr = std::shared_ptr<NonUniformLookupTableParams<float>>;
    using LookupPtrConst = std::shared_ptr<const NonUniformLookupTableParams<float>>;

    /** 
     * make a single non-uniform lookup, the "old" way
     * (i.e. parabolic knee)
     */
    static LookupPtr makeCompGainLookup(const Recipe&);

    static float lookup(LookupPtrConst table, float x) {
        return NonUniformLookupTable<float>::lookup(*table, x);
    }

    /**
     * returns a series of points that define a gain curve.
     * removed interior points that are on a straight line.
     */

    class xy {
    public:
        xy(float a, float b) : x(a), y(b) {}
        xy() = default;
        float x = 0;
        float y = 0;
    };
    static xy getGainAtRightInflection(const CompCurves::Recipe& r);
    static xy getGainAtLeftInflection(const CompCurves::Recipe& r);

    static std::function<double(double)> _getContinuousCurve(const CompCurves::Recipe& r, bool useSpline);
    static std::shared_ptr<NonUniformLookupTableParams<double>> makeSplineMiddle(const Recipe&);

    enum class Type {
        ClassicNU,      // from Comp - non uniform
        ClassicLin,     // sped up version of classic using uniform
        SplineLin,      // spline for knee, fast lookup
    };

    static std::function<float(float)> getLambda(const Recipe&, Type);

    static double bottomOfKneeVin(const Recipe& r) {
        const double bottomOfKneeDb = -r.kneeWidth / 2;
        const double bottomOfKneeVin_ = float(AudioMath::gainFromDb(bottomOfKneeDb));
        return bottomOfKneeVin_;
    }
    static double topOfKneeVin(const Recipe& r) {
        const double topOfKneeDb = r.kneeWidth / 2;
        const double topOfKneeVin_ = float(AudioMath::gainFromDb(topOfKneeDb));
        return topOfKneeVin_;
    }

    static std::function<double(double)> getKneeInterpolator(const Recipe& r) {
        double x0 = 0;
        double x1 = 1;
        double y0 = bottomOfKneeVin(r);
        double y1 = topOfKneeVin(r);

        double a = (y1 - y0) / (x1 - x0);
        double b = y0; 
        return [a, b](double x) {
            return a * x + b;
        };
    }

private:
    static xy addLeftSideCurve(LookupPtr, const Recipe& r);
    static void addRightSideCurve(LookupPtr, const Recipe& r, xy lastPt);
    static void addMiddleCurve(LookupPtr, const Recipe& r, xy lastPt);

 
};