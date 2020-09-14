

#include "MultiLag2.h"
#include "simd.h"

#include "asserts.h"



template <class T>
static void _testMultiLag0()
{
    T l;
   // for (int i = 0; i < size; ++i) {
   //     assertClose(l.get(i), 0, .0001);
   // }
   simd_assertClose(l.get(), float_4(0), .0001);
}

static void testMultiLag0()
{
    _testMultiLag0<MultiLPF2>();
    _testMultiLag0<MultiLag2>();
}



// test that output eventually matches input
template <class T>
static void _testMultiLag1(T& dut)
{

    float_4 input;

    // test each float in the float_4
    for (int n = 0; n < 4; ++n) {

        for (int i = 0; i < 4; ++i) {
            input[i] = (i == n) ? 1.f : 0.f;
        }
        for (int i = 0; i < 10; ++i) {
            dut.step(input);
        }
        for (int i = 0; i < 8; ++i) {
            const float expected = (i == n) ? 1.f : 0.f;
            assertClose(dut.get()[i], expected, .0001);
        }
    }
}

static void testMultiLag1()
{
    MultiLPF2 f;
    f.setCutoff(.4f);
    _testMultiLag1(f);

    MultiLag2 l;
    l.setAttack(.4f);
    l.setRelease(.4f);
    _testMultiLag1(l);
}

void testMultiLag2()
{
 //   testLowpassLookup();
 //   testLowpassLookup2();
 //   testDirectLookup();
  //  testDirectLookup2();

    testMultiLag0();
    testMultiLag1();
  //  testMultiLag2();
  //  testMultiLagDisable();
}