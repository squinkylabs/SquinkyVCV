
#include "CompressorParamHolder.h"
#include "asserts.h"

/*

class CompressorParmHolder {
public:
    unsigned int numChannels = {16};
    unsigned int numBanks = {numChannels / 4};

    float_4 getAttacks(unsigned int bank);
    float_4 getReleases(unsigned int bank);
    float_4 getThresholds(unsigned int bank);
    float_4 getMakeupGain(unsigned int bank);
    float_4 getEnabled(unsigned int bank);
    float_4 wetDryMix(unsigned int bank);
    void getRadio(unsigned int bank, int* ratios);
};
*/

static void test0() {
    assertEQ(CompressorParmHolder::numChannels, 16);
    assertEQ(CompressorParmHolder::numBanks, 4);
}

static void testAttacks(unsigned int bank) {
    assert(bank < CompressorParmHolder::numBanks);

    CompressorParmHolder c;

    float_4 x(.1f, .2f, .3f, .4f);
    bool b = c.setAttacks(bank, x);
    assert(b); 
    simd_assertEQ(c.getAttacks(bank), x);
}

static void testReleases(unsigned int bank) {
    assert(bank < CompressorParmHolder::numBanks);

    CompressorParmHolder c;

    float_4 x(.1f, .2f, .3f, .4f);
    bool b = c.setReleases(bank, x);
    assert(b); 
    simd_assertEQ(c.getReleases(bank), x);
}

void testCompressorParamHolder() {
    test0();
    for (int i = 0; i < CompressorParmHolder::numBanks; ++i) {
        testAttacks(i);
        testReleases(i);
    }
    assert(false);
}