
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

static const float testValues[] = {
    1, 2, 3, 4,
    .1f, .2f, .3f, .4f,
    6, 7, 8, 9,
    -1, -2, -3, -4
};

static void test0() {
    assertEQ(CompressorParmHolder::numChannels, 16);
    assertEQ(CompressorParmHolder::numBanks, 4);
}

static void testAttack(unsigned int channel) {
    assert(channel < CompressorParmHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParmHolder::numBanks);

    CompressorParmHolder c;

    const float x = testValues[channel];

    bool b = c.setAttack(channel, x);
    assert(b);

    assertEQ(c.getAttack(channel), x);

   const unsigned subChannel = channel - (bank * 4);
   assertEQ(c.getAttacks(bank)[subChannel], x);
}


static void testRelease(unsigned int channel) {
    assert(channel < CompressorParmHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParmHolder::numBanks);

    CompressorParmHolder c;

    const float x = testValues[channel] + 1;

    bool b = c.setRelease(channel, x);
    assert(b);

    assertEQ(c.getRelease(channel), x);

    const unsigned subChannel = channel - (bank * 4);
    assertEQ(c.getReleases(bank)[subChannel], x);
}

static void testThreshold(unsigned int channel) {
    assert(channel < CompressorParmHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParmHolder::numBanks);

    CompressorParmHolder c;

    const float x = testValues[channel] - 1;

    bool b = c.setThreshold(channel, x);
    assert(b);

    assertEQ(c.getThreshold(channel), x);

    const unsigned subChannel = channel - (bank * 4);
    assertEQ(c.getThresholds(bank)[subChannel], x);
}


#if 0
static void testMakeupGains(unsigned int bank) {
    assert(bank < CompressorParmHolder::numBanks);

    CompressorParmHolder c;

    float_4 x(.1f, .2f, .3f, .4f);
    bool b = c.setMakeupGains(bank, x);
    assert(b);
    simd_assertEQ(c.getMakeupGains(bank), x);
}

static void testEnableds(unsigned int bank) {
    assert(bank < CompressorParmHolder::numBanks);

    CompressorParmHolder c;

    float_4 x(.1f, .2f, .3f, .4f);
    bool b = c.setEnabled(bank, x);
    assert(b);
    simd_assertEQ(c.getEnableds(bank), x);
}

static void testWetDrys(unsigned int bank) {
    assert(bank < CompressorParmHolder::numBanks);

    CompressorParmHolder c;

    float_4 x(.1f, .2f, .3f, .4f);
    bool b = c.setWetDry(bank, x);
    assert(b);
    simd_assertEQ(c.getWetDryMixs(bank), x);
}

static void testRatios(unsigned int bank) {
    assert(bank < CompressorParmHolder::numBanks);

    CompressorParmHolder c;

    int32_4 x(1, 2, 3, 4);
    bool b = c.setRatio(bank, x);
    assert(b);
    simd_assertEQ(c.getRatios(bank), x);
}
#endif
void testCompressorParamHolder() {
    test0();
    for (int i = 0; i < CompressorParmHolder::numChannels; ++i) {
        testAttack(i);

        testRelease(i);
        testThreshold(i);
       // testMakeupGains(i);
       // testEnableds(i);
        //testWetDrys(i);
        //testRatios(i);

    }
}