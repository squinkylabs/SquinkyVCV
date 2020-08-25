
#include "LookupTableFactory.h"
#include "asserts.h"

#include "simd.h"
#include "SimdBlocks.h"


 //static __m128 twoPi = {_mm_set_ps1(2 * 3.141592653589793238)};
 static float twoPi = 2 * 3.141592653589793238;
static float pi =  3.141592653589793238;


inline double taylor2(double x)
{
    const double xa = ( x - (pi/2.0));
 //  printf("\n**** in taylor2(%f) xa = %f xa**4=%f\n", x, xa, xa*xa*xa*xa);
//   printf("  xa*xa double=%f\n", xa * xa);
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
    float_4 correction = ret * xSquared *  float_4(.02 / .254);
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
        float_4 d = simdSinPiToPi(x);
       // float_4 d2 = simdSinTwoPi(x);

        double err = std::abs(s - d[0]);
        if (err > maxErr) {
            maxErr = err;
            xErr = x;
        }
    }
    printf("-pi .. pi. max err=%f at x=%f\n", maxErr, xErr);
    assertClose(maxErr, 0, .03);
}

static void compare3()
{
    double maxErr = 0;
    double xErr = -100;
    for (double x = 0; x<= twoPi; x += .01) {
        double s = std::sin(x);
        float_4 d = SimdBlocks::sinTwoPi(x);
       // float_4 d2 = simdSinTwoPi(x);

        double err = std::abs(s - d[0]);
        if (err > maxErr) {
            maxErr = err;
            xErr = x;
        }
    }
    printf("0..twopi. max err=%f at x=%f\n", maxErr, xErr);
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
        float d = secondOrder(x);
        printf("y = %f, approx = %f\n", s, d);

       // float_4 d2 = simdSinTwoPi(x);

        double err = std::abs(s - d);
        if (err > maxErr) {
            maxErr = err;
            xErr = x;
        }
    }
    printf("0..twopi. max err=%f at x=%f\n", maxErr, xErr);
    assertClose(maxErr, 0, .03);
}


#if 0
static void both(float x)
{
    printf("\n\n------ both(%f)\n", x);
    float y = std::sin(x);

#if 0
   double y2 = taylor2(x);
   printf("back in both, taylor 2 ret %f\n", y2);
   double y3 = -taylor2n(x);
#endif

    printf("sin(%f)=%f\n", x, y); 
    printf("  taylor combined = %f\n", t2(x));
    printf("  ret from simdSinPiToPi() = %s\n", toStr(simdSinPiToPi(x)).c_str());
    printf("  ret from simdSinTwoPi() = %s\n", toStr(SimdBlocks::sinTwoPi(x)).c_str());
    printf ("\n"); 
}


static void test0()
{
 //   both (0);
    both( pi / 2.0);
    both (0);
  //  both( pi / 4.0);
 //   both (pi);
 //   printf("\n");
    both( -pi / 2.0);
    both( 2 * pi);
//    both( -pi / 4.0);
 //   both (-pi);

  //  both(3.138407);
    #if 0
    both(0);
    both(.1);
    both(-.1);
    both(.5);
    both (.25);
    both (-.25);
    both (-.5);
    printf("\n");
    both( pi / 2.0);
     both( -pi / 2.0);

    both(pi);
    both(-pi);
    both(twoPi);
    both(3);
    both(1);
    both(2);
#endif

}
#endif

void testSimdLookup()
{
   // test0();
    compare();
    compare3();
  //  compare2();
 //   compareSecond();
}