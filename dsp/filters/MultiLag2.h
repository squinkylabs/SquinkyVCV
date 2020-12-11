#pragma once

// #ifndef _MSC_VER
#if 1
#include "simd.h"
#include "SimdBlocks.h"

#include "LookupTable.h"
#include "LowpassFilter.h"

/**
 * MultiLag2 is based on MultiLag, but uses VCV SIMD library
 */
class MultiLPF2 {
public:
    float_4 get() const { return memory; }
    void step(float_4 input);
    
    /**
     * set cutoff, normalized freq
     */
    void setCutoff(float);
private:
    float_4 l = 0;
    float_4 k = 0;
    float_4 memory = 0;
    std::shared_ptr<NonUniformLookupTableParams<float>> lookup = makeLPFilterL_Lookup<float>();
};

/**
 * z = _z * _l + _k * x;
 */
inline void MultiLPF2::step(float_4 input)
{
    float_4 temp = input * k;
    memory *= l;
    memory += temp;
}

inline void MultiLPF2::setCutoff(float fs)
{
    assert(fs > 00 && fs < .5);

    float ls = NonUniformLookupTable<float>::lookup(*lookup, fs);
    float ks = LowpassFilter<float>::computeKfromL(ls);
    k = float_4(ks);
    l = float_4(ls);
}


///////////////////////////////////////////////////////////////////

class MultiLag2 {
public:
    float_4 get() const;
    void step(float_4 input);

    /**
     * attack and release specified as normalized frequency (LPF equivalent)
     */
    void setAttack(float);
    void setRelease(float);

    void setEnable(bool);
    void setInstantAttack(bool);

    float_4 _memory() const;
private:
    float_4 memory = 0;
    float_4 lAttack = 0;
    float_4 lRelease = 0;
    float_4 instant = 0;

    std::shared_ptr<NonUniformLookupTableParams<float>> lookup = makeLPFilterL_Lookup<float>();
    bool enabled = true;
};

inline void MultiLag2::setInstantAttack(bool b) {
    // tortured way to may a simd boolean mask - make this a function!
    if (!b) {
        instant = 0;
    } else {
        instant = (float_4(1) > float_4(0));
    }
    simd_assertMask(instant);
}

inline void MultiLag2::setEnable(bool b) {
    enabled = b;
}

inline float_4 MultiLag2::_memory() const {
    return memory;
}
/**
 * z = _z * _l + _k * x;
 */
inline void MultiLag2::step(float_4 input)
{
  //  printf("--step, input = %s\n", toStr(input).c_str());
    if (!enabled) {
        memory = input;
        return;
    }

    const float_4 isAttack = input >= memory;
    float_4 l = SimdBlocks::ifelse(isAttack, lAttack, lRelease);
    float_4 k = float_4(1) - l;
  //  printf("l=%s k=%s\n", toStr(l).c_str(), toStr(k).c_str());
    float_4 temp = input * k;
    float_4 laggedMemory = temp + memory * l;
   // memory *= l;
  //  memory += temp;
    const float_4 isInstantAttack = isAttack & instant;
 //   printf("in step. isInsta = %s isAtt = %s\n", toStr(isInstantAttack).c_str(), toStr(isAttack).c_str());
    memory = SimdBlocks::ifelse(isInstantAttack, input, laggedMemory);
 //   printf("lagged mem = %s, final mem = %s\n", toStr(laggedMemory).c_str(), toStr(memory).c_str());

}

inline float_4 MultiLag2::get() const
{
    return memory;
}

inline void MultiLag2::setAttack(float fs)
{
  //  printf("ML set attack %f\n", fs); fflush(stdout);
    assert(fs > 00 && fs < .5);
  //  float ls = NonUniformLookupTable<float>::lookup(*lookup, fs);
    float ls = LowpassFilter<float>::computeLfromFs(fs);
    {
    //    printf("fs = %f, l(comP) = %f, look=%f\n", fs, ls, NonUniformLookupTable<float>::lookup(*lookup, fs));
    }
    lAttack = float_4(ls);
}

inline void MultiLag2::setRelease(float fs)
{
    assert(fs > 00 && fs < .5);
    //float ls = NonUniformLookupTable<float>::lookup(*lookup, fs);
    float ls = LowpassFilter<float>::computeLfromFs(fs);
    lRelease = float_4(ls);
}
#endif
