
#include "LookupTableFactory.h"
#include "asserts.h"

#include "simd.h"
#include "SimdBlocks.h"


 //static __m128 twoPi = {_mm_set_ps1(2 * 3.141592653589793238)};
 static float twoPi = 2 * 3.141592653589793238f;
static float pi =  3.141592653589793238f;


inline double taylor2(double x)
{
    const double xa = ( x - (pi/2.0));

    double ret = 1;
    ret -= xa * xa / 2.0;

    const double lastTerm =  xa * xa * xa * xa / 24.f;
    ret += lastTerm;
    const double correction = - lastTerm * .02 / .254;
    ret += correction;
    return ret;
}

/*
 * only accurate for -pi <= x <= pi
 */
inline float_4 simdSinPiToPi(float_4 _x) {
    float_4 xneg = _x < float_4::zero();
    float_4 xOffset = SimdBlocks::ifelse(xneg, float_4(pi / 2.f), float_4(-pi  / 2.f));
    xOffset += _x;
    float_4 xSquared = xOffset * xOffset;
    float_4 ret = xSquared * float_4(1.f / 24.f);
    float_4 correction = ret * xSquared *  float_4(float(.02 / .254));
    ret += float_4(-.5);
    ret *= xSquared;
    ret += float_4(1.f);

    ret -= correction;
    return SimdBlocks::ifelse(xneg, -ret, ret);    
}

#if 0
/*
 * only accurate for 0 <= x <= two
 */
inline float_4 simdSinTwoPi(float_4 _x) {
    _x -= SimdBlocks::ifelse((_x > float_4(pi)), float_4(twoPi), float_4::zero()); 

    float_4 xneg = _x < float_4::zero();
    float_4 xOffset = SimdBlocks::ifelse(xneg, float_4(pi / 2.f), float_4(-pi  / 2.f));
    xOffset += _x;
    float_4 xSquared = xOffset * xOffset;
    float_4 ret = xSquared * float_4(1.f / 24.f);
    float_4 correction = ret * xSquared *  float_4(.02 / .254);
    ret += float_4(-.5);
    ret *= xSquared;
    ret += float_4(1.f);

    ret -= correction;
    return SimdBlocks::ifelse(xneg, -ret, ret);    
}
#endif


inline double taylor2n(double x)
{
    const double xa = ( x +(pi/2.0));
    double ret = 1;
    ret -= xa * xa / 2.0;

    const double lastTerm =  xa * xa * xa * xa / 24.0;
    ret += lastTerm;
    const double correction = - lastTerm * .02 / .254;
    ret += correction;
    return ret;
}

inline double t2(double x)
{
    return (x < 0) ? -taylor2n(x) : taylor2(x);
}

static void compare()
{
    double maxErr = 0;
    double xErr = -100;
    for (double x = -pi; x<= pi; x += .01) {
        double s = std::sin(x);
        float_4 d = simdSinPiToPi(float(x));
       // float_4 d2 = simdSinTwoPi(x);

        double err = std::abs(s - d[0]);
        if (err > maxErr) {
            maxErr = err;
            xErr = x;
        }
    }
    // printf("-pi .. pi. max err=%f at x=%f\n", maxErr, xErr);
    assertClose(maxErr, 0, .03);
}

static void compare3()
{
    double maxErr = 0;
    double xErr = -100;
    for (double x = 0; x<= twoPi; x += .01) {
        double s = std::sin(x);
        float_4 d = SimdBlocks::sinTwoPi(float(x));
       // float_4 d2 = simdSinTwoPi(x);

        double err = std::abs(s - d[0]);
        if (err > maxErr) {
            maxErr = err;
            xErr = x;
        }
    }
    // printf("0..twopi. max err=%f at x=%f\n", maxErr, xErr);
    assertClose(maxErr, 0, .03);
}

float secondOrder(float x)
{
    // c=3/4=0.75
    const float c = 0.75f;

    return (2 - 4 * c) * x * x + c + x;
}

static void compareSecond()
{
    double maxErr = 0;
    double xErr = -100;
    // x += .01
    for (double x = 0; x<= twoPi; x += .2) {
        double s = std::sin(x);
      //  float_4 d = SimdBlocks::sinTwoPi(x);
        float d = secondOrder(float(x));
        // printf("y = %f, approx = %f\n", s, d);

       // float_4 d2 = simdSinTwoPi(x);

        double err = std::abs(s - d);
        if (err > maxErr) {
            maxErr = err;
            xErr = x;
        }
    }
    // printf("0..twopi. max err=%f at x=%f\n", maxErr, xErr);
    assertClose(maxErr, 0, .03);
}

void testSimdLookup()
{
   // test0();
    compare();
    compare3();
  //  compare2();
 //   compareSecond();
}