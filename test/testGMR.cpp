
#include "asserts.h"
#include "GMR.h"
#include "TestComposite.h"

#include <set>

using G = GMR<TestComposite>;

// test that we get triggers out
static void test0()
{
    SQINFO("-------------- testGMR-0 -------------");
    G gmr;
    std::set<float> data;

    gmr.setSampleRate(44100);
    gmr.init();

    for (int i = 0; i < 10; ++i) {

        gmr.inputs[G::CLOCK_INPUT].setVoltage(0, 0);
        for (int i = 0; i < 100; ++i) {
            gmr.step();
            float out = gmr.outputs[G::TRIGGER_OUTPUT].getVoltage(0);
            data.insert(out);
        }
        gmr.inputs[G::CLOCK_INPUT].setVoltage(10, 0);
        for (int i = 0; i < 100; ++i) {
            gmr.step();
            float out = gmr.outputs[G::TRIGGER_OUTPUT].getVoltage(0);
            data.insert(out);
        }
    }

    assert(data.find(cGateOutHi) != data.end());
    assert(data.find(cGateOutLow) != data.end());
    assertEQ(data.size(), 2);
  //  assert(false);
}


static void test1()
{
    SQINFO("-------------- testGMR2-1 -------------");
    G gmr;
   // std::set<float> data;
  //  std::map<
    int transitions = 0;

    gmr.setSampleRate(44100);
    gmr.init();


    float lastOut = 0;
    for (int i = 0; i < 1000; ++i) {

        gmr.inputs[G::CLOCK_INPUT].setVoltage(0, 0);
        for (int i = 0; i < 100; ++i) {
            gmr.step();

            float out = gmr.outputs[G::TRIGGER_OUTPUT].getVoltage(0);
            if (out != lastOut) {
                transitions++;
                lastOut = out;
            }
          
        }
        gmr.inputs[G::CLOCK_INPUT].setVoltage(10, 0);
        for (int i = 0; i < 100; ++i) {
            gmr.step();
            float out = gmr.outputs[G::TRIGGER_OUTPUT].getVoltage(0);
 
            if (out != lastOut) {
                transitions++;
                lastOut = out;
            }
        }
    }

    SQINFO("transitions = %d", transitions);
    assertGT(transitions, 20);
}


void testGMR()
{
    test0();
    test1();
}