
//#ifndef _MSC_VER
#if 1

#include "asserts.h"
#include "SimdBlocks.h"

static void testAsserts()
{
    simd_assertEQ( float_4(1,2,3,4), float_4(1,2,3,4));
    simd_assertGT( float_4(1.01f,2.01f,3.01f,4.01f), float_4(1,2,3,4));
    simd_assertLT( float_4(1,2,3,4), float_4(1.01f,2.01f,3.01f,4.01f));

    simd_assertBetween( float_4(1,2,3,4),  float_4(.9f,1.9f,2.9f,3.9f),  float_4(1.1f,2.1f,3.1f,4.1f));
}

#if 0
void pr(float_4 x)
{
   // float f = 0;
    float* p = &x[0]; 
    int* pi;

    pi = reinterpret_cast<int*>(p);
    printf("0: %x\n", *pi);
    ++p;
    pi = reinterpret_cast<int*>(p);
    printf("1: %x\n", *pi);

++p;
     pi = reinterpret_cast<int*>(p);
    printf("2: %x\n", *pi);
++p;
     pi = reinterpret_cast<int*>(p);
    printf("3: %x\n", *pi);
    fflush(stdout);
}
#endif

#if 0
inline float_4 fold(float_4 x)
{
     printf("-------------- enter hacked fold \n");

#if 0 // this worked
    float_4 x = _x;
    printf("fold(%s)\n", toStr(x).c_str());
    fflush(stdout);

    // this bad
 //   float_4 mask = x < 0;
    float_4 zero(0);
    float_4 mask = x > zero;
#else
    auto mask = x < 0;
     simd_assertMask(mask);
     float_4 zero(0);
#endif

   printf("fold2(%s)\n", toStr(x).c_str());
    fflush(stdout);

    printf("here are raw x,, zero\n");
    pr(x);
  //  pr(_x);
    pr(zero);

     printf("fold3(%s)\n", toStr(x).c_str());
    fflush(stdout);
 #if 0
    printf("second compare a=%s\n b=%s \n c=%s\n", 
        toStr(x).c_str(),
        toStr(zero).c_str(),
        toStr(mask).c_str()
        );
#endif
    printf("here is mask\n");
    pr(mask);

    fflush(stdout);
    simd_assertMask(mask);
    return zero;
}
#endif

static void testMask()
{
    float_4 m = float_4::mask();
    float_4 nm = ~m;

    float_4 a(1000,2,-30000000,4);
    float_4 b(12345,8793,30000000,-4343);

    float_4 x = SimdBlocks::ifelse (m, a, b);
    float_4 y = SimdBlocks::ifelse (m, b, a);

    assert(isMask(float_4::mask()));
    assert(isMask(float_4::zero()));

    int temp = movemask(m);

    assert(isMask(m));
    assert(isMask(nm));
    assert(isMask( float_4(m[0], nm[1], m[2], nm[3]) ));

    assert(!isMask(float_4(1000,2,-30000000,4)) );
    assert(!isMask(float_4(12345,8793,30000000,-4343)) );
    assert(isMask( float_4(0)));

    float_4 one(1);
    float_4 zero(0);
    float_4 c1 = one > zero;
    simd_assertMask(c1);

    c1 = one > 0;
    simd_assertMask(c1);

float_4 xx(0);
  SimdBlocks::fold(xx);
}



static void testMaskInt()
{
    int32_4 m = int32_4::mask();
    int32_4 nm = ~m;  
    assert(isMask(m));
    assert(isMask(nm));
}

static void testMinMax()
{
    float_4 x = SimdBlocks::min( float_4(1,2,3,4), float_4(2, 1, 5, 3));
    assertEQ(x[0], 1);
}


// If we ever want to use it, we should move these functions
float_4 deinterlace_low(const float_4& x, const float_4& y) {
    const int mask =  (0 << 0) | (2 << 2) | (0 << 4) | (2 << 6);
    float_4 ret = _mm_shuffle_ps(x.v, y.v, mask);
    fflush(stdout);
    return ret;
}

float_4 deinterlace_high(const float_4& x, const float_4& y) {
    const int mask =  (1 << 0) | (3 << 2) | (1 << 4) | (3 << 6);
    float_4 ret = _mm_shuffle_ps(x.v, y.v, mask);
    fflush(stdout);
    return ret;
}
static void testDeInterleaveLow()
{
    float_4 x(1,2,3, 4);
    float_4 y(110, 120, 130, 140);

    assertEQ(x[0], 1);
    float_4 z = deinterlace_low(x, y);

    assertEQ(z[0], x[0]);
    assertEQ(z[1], x[2]);
    assertEQ(z[2], y[0]);
    assertEQ(z[3], y[2]);
}

static void testDeInterleaveHigh()
{
    float_4 x(1,2,3, 4);
    float_4 y(110, 120, 130, 140);

    assertEQ(x[0], 1);
    float_4 z = deinterlace_high(x, y);

    assertEQ(z[0], x[1]);
    assertEQ(z[1], x[3]);
    assertEQ(z[2], y[1]);
    assertEQ(z[3], y[3]);
}

void testSimd()
{
    testAsserts();
    testMask();
    testMaskInt();
    testMinMax();
    testDeInterleaveLow();
    testDeInterleaveHigh();
}
#endif