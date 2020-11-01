#pragma once

#include <utility>

class SplineBase
{
public:
    using point = std::pair<double, double>; 
    virtual point renderPoint(double t) = 0;
};


class HermiteSpline : public SplineBase
{
public:
    HermiteSpline( 
       point p0,
       point p1,
       point tangent0,
       point tangent1);

       
    std::pair<double, double> renderPoint(double t) override;

private:
    point p0;
    point p1;
    point m0;
    point m1;

};