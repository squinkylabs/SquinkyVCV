#pragma once

#include <utility>

class SplineBase
{
public:
    virtual std::pair<double, double> renderPoint(double t) = 0;
};


class HermiteSpline : public SplineBase
{
public:
    HermiteSpline();
    virtual std::pair<double, double> renderPoint(double t);
};