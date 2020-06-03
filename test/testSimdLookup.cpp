
#include "LookupTableFactory.h"
#include "asserts.h"

#include "simd.h"
#include "SimdBlocks.h"




#if 0

inline float_4 lookupSimd(const LookupTableParams<float>& params, float_4 input, bool allowOutsideDomain)
{
    input = rack::simd::clamp(input, float_4(params.xMin), float_4(params.xMax));
    assert(params.isValid());
    

    // need to scale by bins
    float_4 scaledInput = input * float_4(params.a) + float_4(params.b);
    assert(params.a != 0);


   // int input_int = cvtt(&scaledInput);
    float_4 input_trunc = rack::simd::trunc(scaledInput);
    float_4 input_float = scaledInput - input_trunc;

    simd_assertGE(input_float, float_4::zero());
    simd_assertLE(input_float, float_4(1));

    int32_4 input_int = input_trunc;
 //   printf("trunc = %s, int=%s\n", toStr(input_trunc).c_str(), toStr(input_int).c_str());

    // TODO: assert in integer?
    simd_assertGE(input_trunc, float_4::zero());
    simd_assertLE(input_trunc, float_4(params.numBins_i));

    float_4 value;
    float_4 slope;
    for (int i=0; i<3; ++i) {
        float* entry = params.entries + (2 * input_int[i]);
        value[i] = entry[0];
        slope[i] = entry[1];
    }

    float_4 ret = value + slope * input_float;
    return ret;
}

static void compare(const LookupTableParams<float>& params, float input)
{
    float_4 x4 = lookupSimd(params, float_4(input), false);
    float xf = LookupTable<float>::lookup(params, input, false);
    assertClose(x4[0], xf, .01);  
}


static void test0()
{
  //  LookupTableParams<float_4> p4;
    LookupTableParams<float> pf;
    LookupTableFactory<float>::makeAudioTaper(pf);
  //  copyLookupParams(p4, pf);

    compare(pf, 0);
    compare(pf, 1.f);
    compare(pf, .5f);
/*
    float_4 x4;
    float xf;

    x4 = lookupSimd(pf, float_4(0), false);
    xf = LookupTable<float>::lookup(pf, 0.f, false);
    assertClose(x4[0], xf, .01);
    */
}
#endif

int fact(int x) {
    if (x == 1) {
        return 1;
    } else {
        return x * fact(x-1);
    }
}

static void testFact()
{
    printf("fact 7 = %d\n", fact(7));
    printf("fact 5 = %d\n", fact(5));
    printf("fact 3 = %d\n", fact(3));
}

 //static __m128 twoPi = {_mm_set_ps1(2 * 3.141592653589793238)};
 static float twoPi = 2 * 3.141592653589793238;
static float pi =  3.141592653589793238;


inline double taylor2(double x)
{
    const double xa = ( x - (pi/2.0));
   printf("\n**** in taylor2(%f) xa = %f xa**4=%f\n", x, xa, xa*xa*xa*xa);
   printf("  xa*xa double=%f\n", xa * xa);
    double ret = 1;
    ret -= xa * xa / 2.0;

    const double lastTerm =  xa * xa * xa * xa / fact(4);
    ret += lastTerm;
    const double correction = - lastTerm * .02 / .254;
    ret += correction;

  //  printf("last term= %f\n", xa * xa * xa * xa / fact(4));
  //  printf("taylor2 ret %f\n", ret);
    return ret;
}

// simple minded way
#if 0
inline float_4 simdSin(float_4 _x) {
    float_4 xneg = _x < float_4::zero();
    float_4 xOffset = SimdBlocks::ifelse(xneg, float_4(pi / 2.f), float_4(-pi  / 2.f));
    xOffset += _x;
    float_4 xSquared = xOffset * xOffset;
    printf("\n*** in simdsin(%s) xsq=%s\n xoff=%s\n",
        toStr(_x).c_str(),
        toStr(xSquared).c_str(),
        toStr(xOffset).c_str());
        

    float_4 ret(1);
    ret -= xSquared / float_4(2);

    printf("fact(4) = %s\n", toStr(float_4(fact(4))).c_str());
    float_4 lastTerm = xSquared * xSquared / float_4(fact(4));
    ret += lastTerm;
  //  float_4 ret = xSquared * float_4(1.f / 24.f);
   // ret += float_4(.5);
   // ret *= xSquared;
   // ret *= float_4(-1);
   // ret += float_4(1.f);


// no term correction, so at zero off my .02
   return  SimdBlocks::ifelse(xneg, -ret, ret);
  //  return ret;
    
}
#endif

#if 1 // original way
inline float_4 simdSin(float_4 _x) {
    float_4 xneg = _x < float_4::zero();
    float_4 xOffset = SimdBlocks::ifelse(xneg, float_4(pi / 2.f), float_4(-pi  / 2.f));
    xOffset += _x;
    float_4 xSquared = xOffset * xOffset;
#if 0
    printf("\n*** in simdsin(%s) xsq=%s\n xoff=%s\n",
        toStr(_x).c_str(),
        toStr(xSquared).c_str(),
        toStr(xOffset).c_str());
#endif

    float_4 ret = xSquared * float_4(1.f / 24.f);
    ret += float_4(-.5);
    ret *= xSquared;
    ret += float_4(1.f);

    // no term correction, so at zero off my .02
   return  SimdBlocks::ifelse(xneg, -ret, ret);
    
}
#endif

inline double taylor2n(double x)
{
    const double xa = ( x +(pi/2.0));
 //   printf("taylor2n(%f) xa = %f xa**4=%f\n", x, xa, xa*xa*xa*xa);
    double ret = 1;
    ret -= xa * xa / 2.0;

    const double lastTerm =  xa * xa * xa * xa / fact(4);
    ret += lastTerm;
    const double correction = - lastTerm * .02 / .254;
    ret += correction;

   // printf("last term= %f\n", xa * xa * xa * xa / fact(4));
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
        float_4 d = simdSin(x);

        double err = std::abs(s - d[0]);
        if (err > maxErr) {
            maxErr = err;
            xErr = x;
        }
    }
    printf("max err=%f at x=%f\n", maxErr, xErr);
}


static void compare2()
{
    double maxErr = 0;
    double xErr = -100;
    for (double x = -pi; x<= pi; x += .01) {
        double s = std::sin(x);
        double d = t2(x);

        double err = std::abs(s - d);
        if (err > maxErr) {
            maxErr = err;
            xErr = x;
        }
    }
    printf("max simd err=%f at x=%f\n", maxErr, xErr);
}

static void both(float x)
{
    printf("\n\n------ both(%f)\n", x);
    float y = std::sin(x);

   double y2 = taylor2(x);
   printf("back in both, taylor 2 ret %f\n", y2);
   double y3 = -taylor2n(x);

    printf("sin(%f)=%f, taylor = %f | %f\n", x, y, y2, y3); 
    printf("  combined = %f\n", t2(x));
    printf("  ret from simdSin() = %s\n", toStr(simdSin(x)).c_str());
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

void testSimdLookup()
{
  //  testFact();
    test0();
  //  compare();
  //  compare2();

    fflush(stdout);
    assert(false);
}