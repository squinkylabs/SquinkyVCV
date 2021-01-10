
#include "Samp.h"

#include "TestComposite.h"
#include "asserts.h"

#include <ctime>
#include <ratio>
#include <chrono>

static void test1()
{
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    Samp<TestComposite> s;
    TestComposite::ProcessArgs args;

    s.init();

    const char* p = R"foo(D:\samples\UprightPianoKW-small-SFZ-20190703\UprightPianoKW-small-20190703.sfz)foo";
    s.setNewSamples(p);
    while (true) {
        if (s._sampleLoaded()) {
            return;
        }

        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        double second = time_span.count();
        assertLE(second, 10);

        s.process(args);
    }
    assert(false);
}

void testWavThread()
{
    test1();
}