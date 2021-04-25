
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

static void testInitValues() {
    const CompressorParmHolder p;
    for (int channel = 0; channel < CompressorParmHolder::numChannels; ++channel) {
        const int bank = channel / 4;
        assertEQ(0, p.getAttack(channel));
        simd_assertEQ(float_4::zero(), p.getAttacks(bank));

        assertEQ(0, p.getRelease(channel));
        simd_assertEQ(float_4::zero(), p.getReleases(bank));

        assertEQ(false, p.getEnabled(channel));
        simd_assertEQ(SimdBlocks::maskFalse(), p.getAttacks(bank));
        
    }
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

   for (int i=0; i < CompressorParmHolder::numChannels; ++i) {
       if (i != channel) {
           assertNE(c.getAttack(i), x);
       }
   }
}


static void testRelease(unsigned int channel) {
    assert(channel < CompressorParmHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParmHolder::numBanks);

    CompressorParmHolder c;

    const float x = testValues[channel] + 100;

    bool b = c.setRelease(channel, x);
    assert(b);

    assertEQ(c.getRelease(channel), x);

    const unsigned subChannel = channel - (bank * 4);
    assertEQ(c.getReleases(bank)[subChannel], x);

    for (int i = 0; i < CompressorParmHolder::numChannels; ++i) {
        if (i != channel) {
            assertNE(c.getRelease(i), x);
        }
    }
}

static void testThreshold(unsigned int channel) {
    assert(channel < CompressorParmHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParmHolder::numBanks);

    CompressorParmHolder c;

    const float x = testValues[channel] - 10;

    bool b = c.setThreshold(channel, x);
    assert(b);

    assertEQ(c.getThreshold(channel), x);

    const unsigned subChannel = channel - (bank * 4);
    assertEQ(c.getThresholds(bank)[subChannel], x);

    for (int i = 0; i < CompressorParmHolder::numChannels; ++i) {
        if (i != channel) {
            assertNE(c.getThreshold(i), x);
        }
    }
}

static void testMakeupGain(unsigned int channel) {
    assert(channel < CompressorParmHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParmHolder::numBanks);

    CompressorParmHolder c;

    const float x = testValues[channel] - 1.23f;

    bool b = c.setMakeupGain(channel, x);
    assert(b);

    assertEQ(c.getMakeupGain(channel), x);

    const unsigned subChannel = channel - (bank * 4);
    assertEQ(c.getMakeupGains(bank)[subChannel], x);

    for (int i = 0; i < CompressorParmHolder::numChannels; ++i) {
        if (i != channel) {
            assertNE(c.getMakeupGain(i), x);
        }
    }
}


void testCompressorParamHolder() {
    test0();
    testInitValues();
    for (int i = 0; i < CompressorParmHolder::numChannels; ++i) {
        testAttack(i);

        testRelease(i);
        testThreshold(i);
        testMakeupGain(i);
       // testEnabled(i);
        //testWetDry(i);
        //testRatio(i);

    }
}