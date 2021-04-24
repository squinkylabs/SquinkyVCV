
#include "simd.h"
#include "MeasureTime.h"
#include "asserts.h"

double overheadInOut = 0;
double overheadOutOnly = 0;

static void setup()
{
#ifdef _DEBUG
//    assert(false);  // don't run this in debug
#endif
    TestBuffers<float>::doInit();
    double d = .1;
    const double scale = 1.0 / RAND_MAX;
    overheadInOut = MeasureTime<float>::run(0.0, "test1 (do nothing i/o)", [&d, scale]() {
        return TestBuffers<float>::get();
        }, 1);

    overheadOutOnly = MeasureTime<float>::run(0.0, "test1 (do nothing oo)", [&d, scale]() {
        return 0.0f;
        }, 1);

}

void initPerf()
{
    printf("initializing perf...\n");
    assert(overheadInOut == 0);
    assert(overheadOutOnly == 0);
    setup();
    assert(overheadInOut > 0);
    assert(overheadOutOnly > 0);
}