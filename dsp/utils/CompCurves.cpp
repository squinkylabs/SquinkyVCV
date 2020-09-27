

#include "CompCurves.h"
#include <functional>



static std::function<float(float)> continuousGainFunction(const CompCurves::Recipe& r)
{
    return [](float x) {
        return 0.f;
    };
}

std::vector<CompCurves::xy> CompCurves::makeCrudeCompGainTable(const Recipe& r)
{
    auto func = continuousGainFunction(r);
    return std::vector<CompCurves::xy>();
}