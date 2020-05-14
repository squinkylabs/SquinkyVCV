#pragma once

#include <simd/vector.hpp>
#include <simd/functions.hpp>

using float_4 = rack::simd::float_4;
using int32_4 = rack::simd::int32_4;

class SimdBlocks
{
public:
    static float_4 fold(float_4);
};


std::string toStr(const float_4& x) {
    std::stringstream s;
    s << x[0] << ", " << x[1] << ", " << x[2] << ", " << x[3];
    return s.str();
}


std::string toStr(const int32_4& x) {
    std::stringstream s;
    s << x[0] << ", " << x[1] << ", " << x[2] << ", " << x[3];
    return s.str();
}

/*
 static inline float fold(float x)
    {
        float fold;
        const float bias = (x < 0) ? -1.f : 1.f;
        int phase = int((x + bias) / 2.f);
        bool isEven = !(phase & 1);
        if (isEven) {
            fold = x - 2.f * phase;
        } else {
            fold = -x + 2.f * phase;
        }
        return fold;
    }
*/




inline float_4 SimdBlocks::fold(float_4 x)
{
    printf("***************\n");
    float xx = x[0];
    const float biasx = (xx < 0) ? -1.f : 1.f;
    printf("x = %f, bias = %f\n", xx, biasx);
    auto mask = x < 0;
    float_4 bias = ifelse(mask, -1, 1);
    printf("x= %s, bias = %s\n\n", toStr(x).c_str(), toStr(bias).c_str()); fflush(stdout);

    int phasex = int((xx + biasx) / 2.f);
    printf("phase = %d\n", phasex);
    float_4 temp =(x + bias) / 2.f;
    int32_4 phase(temp);
    printf("temp = %s, phase=%s\n\n", toStr(temp).c_str(), toStr(phase).c_str());

    bool isEvenx = !(phasex & 1);
    printf("isEven = %d ph&1 = %d\n", isEvenx, phasex&1);
    int32_4 one(1);
    //int32_4 isEven = 0 - (phase & one);

    // is this a single instruction?

    // first try: gives 0, 1, but we want 0 / 11111111....

// trying to get some logic going
   //  int32_4 phAone = rack::simd::movemaskInverse(phase);

   /// sse_assertBool(phAone)
  // assert(phAone[0] == 0 || phAone[0] == 0xffffffff);
    int32_4 isEven = one ^ (phase & one);
    printf("first try isEven = %s\n", toStr(isEven).c_str());
    isEven = (isEven > 0);

   // int32_4 isEven= 
   // assert(isEven[0] == 0 || isEven[0] == 0xffffffff);
   simd_assertMask(isEven);

    printf("is even = %s one =%s phase&one = %s\n\n", 
        toStr(isEven).c_str(), 
        toStr(one).c_str(),
        toStr(phase & one).c_str());

    float foldx;
     if (isEvenx) {
            foldx = xx - (2.f * phasex);
            printf("is even\n");
     } else {
            foldx = -xx + 2.f * phasex;
            printf("is odd\n");
    }
    printf("fold = %f\n", foldx);

        // TODO: can optimize! both sides are mirrors
    float_4 evenFold = x - (2.f * phase);
    float_4 oddFold = (0 - x) + (2.f * phase);
    printf("even fold = %s\n", toStr(evenFold).c_str());
    printf("odd fold = %s\n", toStr(oddFold).c_str());
    auto ret = ifelse(isEven, evenFold, oddFold);
    printf("fold = %s\n\n", toStr(ret).c_str());

    return ret;
   // return x;
}