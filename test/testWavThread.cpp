
#include "Samp.h"

#include "TestComposite.h"
#include "asserts.h"

#include <ctime>
#include <ratio>
#include <chrono>

static void test1()
{
    // std::this_thread::sleep_for(std::chrono::seconds(20));
  //  std::chrono::steady_clock
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    Samp<TestComposite> s;
    TestComposite::ProcessArgs args;

    s.init();

    s.setNewSamples("");
    while (true) {
        if (s._sampleLoaded()) {
            return;
        }

        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        double second = time_span.count();
        assertLE(second, 2);

        s.process(args);
    }
    assert(false);
}

void testWavThread()
{
    test1();
}