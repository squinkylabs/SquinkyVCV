

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
   //  printf("at start of test dut=%s\n", toStr(dut.get()).c_str()); 

    // test each float in the float_4
    for (int n = 0; n < 4; ++n) {

        for (int i = 0; i < 4; ++i) {
            input[i] = (i == n) ? 1.f : 0.f;
        }
        for (int i = 0; i < 10; ++i) {
            dut.step(input);
        }
      // printf("after stepping n=%d dut=%s\n", n, toStr(dut.get()).c_str()); 
        for (int i = 0; i < 4; ++i) {
            const float expected = (i == n) ? 1.f : 0.f;
            assertClose(dut.get()[i], expected, .0001);
        }
    }
}

static void testMultiLag1()
{
    printf("testing lpf\n");
    MultiLPF2 f;
    f.setCutoff(.4f);
    _testMultiLag1(f);

    printf("testing lag\n");
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