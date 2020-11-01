
#include "SplineRenderer.h"

#include <assert.h>
#include <cmath>


 HermiteSpline::HermiteSpline(
     point p0,
     point p1,
     point tangent0,
     point tangent1) : p0(p0), p1(p1), m0(tangent0), m1(tangent1)
 {

 }
    
 HermiteSpline::point HermiteSpline::renderPoint(double t)
{

    // return std::make_pair(0.0, 0.0);
    point ret;

    const double kp0 = 2 * std::pow(t, 3) - 3 * std::pow(t, 2) + 1; 
    const double km0 = std::pow(t, 3) - 2 * std::pow(t, 2) + t;
    const double kp1 = -2 * std::pow(t, 3) + 3 * std::pow(t, 2);
    const double km1 = std::pow(t, 3) - std::pow(t, 2);


    ret.first = 
        kp0 * p0.first +
        km0 * m0.first +
        kp1 * p1.first +
        km1 * m1.first;

    ret.second = 
        kp0 * p0.second +
        km0 * m0.second +
        kp1 * p1.second +
        km1 * m1.second;
    return ret;
}