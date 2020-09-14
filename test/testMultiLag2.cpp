

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

void testMultiLag2()
{
 //   testLowpassLookup();
 //   testLowpassLookup2();
 //   testDirectLookup();
  //  testDirectLookup2();

    testMultiLag0();
 //   testMultiLag1();
  //  testMultiLag2();
  //  testMultiLagDisable();
}