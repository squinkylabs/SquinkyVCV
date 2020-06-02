
#include "LookupTableFactory.h"
#include "asserts.h"

#include "simd.h"


/*
       entries = (T *) malloc((bins + 1) * 2 * sizeof(T));
        numBins_i = bins;
        */
    #if 0
void copyLookupParams(LookupTableParams<float_4>& out, const LookupTableParams<float>& in)
{
    out.alloc(in.numBins_i);
    out.xMin = float_4(in.xMin);
    out.xMax = float_4(in.xMax);
    out.numBins_i = in.numBins_i;

    const int numEntries = 2 * (in.numBins_i + 1);
    for (int i=0; i< numEntries; ++i) {
        float x = in.entries[i];
        out.entries[i] = float_4(x);
    }
}
#endif



inline float_4 lookupSimd(const LookupTableParams<float>& params, float_4 input, bool allowOutsideDomain)
{
    #if 0
    assert(allowOutsideDomain || (input >= params.xMin && input <= params.xMax));
                                                            // won't happen in the field,
                                                            // as assertions are disabled for release.
    #endif
   // input = std::min(input, params.xMax);
   // input = std::max(input, params.xMin);

    input = rack::simd::clamp(input, float_4(params.xMin), float_4(params.xMax));
    assert(params.isValid());
    
   // assert(input >= params.xMin && input <= params.xMax);

    // need to scale by bins
    float_4 scaledInput = input * float_4(params.a) + float_4(params.b);
    assert(params.a != 0);


   // int input_int = cvtt(&scaledInput);
    float_4 input_trunc = rack::simd::trunc(scaledInput);
    float_4 input_float = scaledInput - input_trunc;

    simd_assertGE(input_float, float_4::zero());
    simd_assertLE(input_float, float_4(1));

    // Unfortunately, when we use float instead of doubles,
    // numeric precision issues get us "out of range". So
    // here we clamp to range. It would be more efficient if we didn't do this.
    // Perhaps the calculation of a and b could be done so this can't happen?
    #if 0
    if (input_float < 0) {
        input_float = 0;
    } else if (input_float > 1) {
        input_float = 1;
    }

    assert(input_float >= 0 && input_float <= 1);
    assert(input_int >= 0 && input_int <= params.numBins_i);
    #endif

    int32_4 input_int = input_trunc;
    printf("trunc = %s, int=%s\n", toStr(input_trunc).c_str(), toStr(input_int).c_str());

    // TODO: assert in integer?
    simd_assertGE(input_trunc, float_4::zero());
    simd_assertLE(input_trunc, float_4(params.numBins_i));

    // x = entry[0] + input_float * entry[1]
    //   = value + fraction * slope
    float_4 value;
    float_4 slope;
    for (int i=0; i<3; ++i) {
        float* entry = params.entries + (2 * input_int[i]);
        value[i] = entry[0];
        slope[i] = entry[1];
    }

    float_4 ret = value + slope * input_float;
    return ret;
   // T * entry = params.entries + (2 * input_int);
  //  T x = entry[0];
  //  x += input_float * entry[1];



   // return x;
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

void testSimdLookup()
{
    test0();
}