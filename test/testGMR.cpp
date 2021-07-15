
#include "asserts.h"
#include "GMR2.h"
#include "TestComposite.h"

#include <set>

using G = GMR2<TestComposite>;

// test that we get triggers out
static void test0()
{
    G gmr;
    std::set<float> data;
    TestComposite::ProcessArgs args;

    gmr.setSampleRate(44100);
    gmr.init();

    for (int i = 0; i < 10; ++i) {

        gmr.inputs[G::CLOCK_INPUT].setVoltage(0, 0);
        for (int i = 0; i < 100; ++i) {
            gmr.process(args);
            float out = gmr.outputs[G::TRIGGER_OUTPUT].getVoltage(0);
            data.insert(out);
        }
        gmr.inputs[G::CLOCK_INPUT].setVoltage(10, 0);
        for (int i = 0; i < 100; ++i) {
            gmr.process(args);
            float out = gmr.outputs[G::TRIGGER_OUTPUT].getVoltage(0);
            data.insert(out);
        }
    }

    assert(data.find(cGateOutHi) != data.end());
    assert(data.find(cGateOutLow) != data.end());
    assertEQ(data.size(), 2);
}

void testGMR2()
{
    test0();
}