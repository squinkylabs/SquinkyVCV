#pragma once

#include <memory>
#include <utility>

#include "asserts.h"

class HermiteSpline;

class SplineBase {
public:
    using point = std::pair<double, double>;
    virtual point renderPoint(double t) = 0;
};

using HermiteSplinePtr = std::shared_ptr<HermiteSpline>;

class HermiteSpline : public SplineBase {
public:
    HermiteSpline(
        point p0,
        point p1,
        point tangent0,
        point tangent1);

    std::pair<double, double> renderPoint(double t) override;

    static HermiteSplinePtr make(int ratio, int width);

private:
    point p0;
    point p1;
    point m0;
    point m1;
};

inline HermiteSplinePtr HermiteSpline::make(int ratio, int width) {
    //  float ratio;
    //  float deltaY;


    if (width == 0) {
        return nullptr;
    }
    assert(width == 12);

    /**
     *  location of the first tangent point. The farther to the right we go, the
     *  more curve we get in the knee. This is the biggest value that still preservers
     *  the monotonic slope values.
     */
    const double secondX = 18;
    const double desiredEndSlope = -1.0 + 1.0 / ratio;
    const double desiredEndValue = 6 * desiredEndSlope;

   // const double y2 = 6 * (-1.0 + 1.0 / ratio);


    double ty = 0;
    switch (ratio) {
    case 2:
        ty = -3;
        break;
    case 4:
        ty = -4.5f;
        break;
    case 8:
        ty = -5.25f;
        break;
    case 20:
        ty = -5.75f;
        break;
    default:
        assert(false);
    }

    HermiteSplinePtr ret = std::make_shared<HermiteSpline>(
        HermiteSpline::point(-6, 0),
        HermiteSpline::point(6, desiredEndValue),
        HermiteSpline::point(secondX, 0),
        HermiteSpline::point(6, ty));

    return ret;
}


#if 0 // old version (.5 2)
inline HermiteSplinePtr HermiteSpline::make(int ratio, int width) {
    //  float ratio;
    //  float deltaY;
    assert(false);  // this is for old curve

    if (width == 0) {
        return nullptr;
    }


    float yCorrection = 0;
    double y2 = 1.0 + 1.0 / ratio;
    assert(width == 12);
    switch (ratio) {
        case 2:
            yCorrection = -1.0;
            break;
        case 4:
            yCorrection = -1.5;
            break;
        case 8:
            yCorrection = -1.75;
            break;
        case 20:
            yCorrection = -1.9f;
            break;
        default:
            assert(false);
    }

    HermiteSplinePtr ret = std::make_shared<HermiteSpline>(
        HermiteSpline::point(.5, .5),
        HermiteSpline::point(2, y2),
        HermiteSpline::point(1.5, 1.5),
        HermiteSpline::point(2, 2 + yCorrection));

    return ret;
}
#endif

