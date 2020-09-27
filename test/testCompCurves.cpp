
#include "CompCurves.h"
#include "asserts.h"
static void testCompCurves0()
{
    // comp ratio of 1 is a straight line - two points
    CompCurves::Recipe r;
    r.minX = -10;
    r.maxX = 10;
    r.ratio = 1;
    r.threshold = 0;

    auto result = CompCurves::makeCrudeCompGainTable(r);
    assertEQ(result.size(), 2);
}

void testCompCurves()
{
    testCompCurves0();
    assert(false);
}