
#include "CompressorParamHolder.h"
#include "asserts.h"

static const float testValues[] = {
    1, 2, 3, 4,
    .1f, .2f, .3f, .4f,
    6, 7, 8, 9,
    -1, -2, -3, -4};

static void test0() {
    assertEQ(CompressorParamHolder::numChannels, 16);
    assertEQ(CompressorParamHolder::numBanks, 4);
}

static void testInitValues() {
    const CompressorParamHolder p;
    for (int channel = 0; channel < CompressorParamHolder::numChannels; ++channel) {
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
    assert(channel < CompressorParamHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParamHolder::numBanks);

    CompressorParamHolder c;

    const float x = testValues[channel];

    c.setAttack(channel, x);
    assertEQ(c.getAttack(channel), x);

    const unsigned subChannel = channel - (bank * 4);
    assertEQ(c.getAttacks(bank)[subChannel], x);

    for (int i = 0; i < CompressorParamHolder::numChannels; ++i) {
        if (i != channel) {
            assertNE(c.getAttack(i), x);
        }
    }
}

static void testRelease(unsigned int channel) {
    assert(channel < CompressorParamHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParamHolder::numBanks);

    CompressorParamHolder c;

    const float x = testValues[channel] + 100;

    c.setRelease(channel, x);

    assertEQ(c.getRelease(channel), x);

    const unsigned subChannel = channel - (bank * 4);
    assertEQ(c.getReleases(bank)[subChannel], x);

    for (int i = 0; i < CompressorParamHolder::numChannels; ++i) {
        if (i != channel) {
            assertNE(c.getRelease(i), x);
        }
    }
}

static void testThreshold(unsigned int channel) {
    assert(channel < CompressorParamHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParamHolder::numBanks);

    CompressorParamHolder c;

    const float x = testValues[channel] - 10;

    c.setThreshold(channel, x);

    assertEQ(c.getThreshold(channel), x);

    const unsigned subChannel = channel - (bank * 4);
    assertEQ(c.getThresholds(bank)[subChannel], x);

    for (int i = 0; i < CompressorParamHolder::numChannels; ++i) {
        if (i != channel) {
            assertNE(c.getThreshold(i), x);
        }
    }
}

static void testMakeupGain(unsigned int channel) {
    assert(channel < CompressorParamHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParamHolder::numBanks);

    CompressorParamHolder c;

    const float x = testValues[channel] - 1.23f;

    c.setMakeupGain(channel, x);

    assertEQ(c.getMakeupGain(channel), x);

    const unsigned subChannel = channel - (bank * 4);
    assertEQ(c.getMakeupGains(bank)[subChannel], x);

    for (int i = 0; i < CompressorParamHolder::numChannels; ++i) {
        if (i != channel) {
            assertNE(c.getMakeupGain(i), x);
        }
    }
}

static void testEnabled(unsigned int channel) {
    assert(channel < CompressorParamHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParamHolder::numBanks);

    CompressorParamHolder c;
    c.setEnabled(channel, true);

    assertEQ(c.getEnabled(channel), true);

    const unsigned subChannel = channel - (bank * 4);
    assertNE(c.getEnableds(bank)[subChannel], SimdBlocks::maskFalse()[0]);
    simd_assertMask(c.getEnableds(bank));

    for (int i = 0; i < CompressorParamHolder::numChannels; ++i) {
        if (i != channel) {
            assert(!c.getEnabled(i));
        }
    }
}

static void testSidechainEnabled(unsigned int channel) {
  //  assert(false);
    assert(channel < CompressorParamHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParamHolder::numBanks);

    CompressorParamHolder c;
    c.setSidechainEnabled(channel, true);

    assertEQ(c.getSidechainEnabled(channel), true);

    const unsigned subChannel = channel - (bank * 4);
    assertNE(c.getSidechainEnableds(bank)[subChannel], SimdBlocks::maskFalse()[0]);
    simd_assertMask(c.getSidechainEnableds(bank));

    for (int i = 0; i < CompressorParamHolder::numChannels; ++i) {
        if (i != channel) {
            assert(!c.getSidechainEnabled(i));
        }
    }
}

static void testWetDry(unsigned int channel) {
    assert(channel < CompressorParamHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParamHolder::numBanks);

    CompressorParamHolder c;

    const float x = testValues[channel] - 1.23f;

    c.setWetDry(channel, x);

    assertEQ(c.getWetDryMix(channel), x);

    const unsigned subChannel = channel - (bank * 4);
    assertEQ(c.getWetDryMixs(bank)[subChannel], x);

    for (int i = 0; i < CompressorParamHolder::numChannels; ++i) {
        if (i != channel) {
            assertNE(c.getWetDryMix(i), x);
        }
    }
}

static void testRatio(unsigned int channel) {
    assert(channel < CompressorParamHolder::numChannels);
    const unsigned int bank = channel / 4;
    assert(bank < CompressorParamHolder::numBanks);

    CompressorParamHolder c;

    // const float x = testValues[channel] - 1.23f;
    const int x = 5;

    c.setRatio(channel, x);

    assertEQ(c.getRatio(channel), x);

    const unsigned subChannel = channel - (bank * 4);
    assertEQ(c.getRatios(bank)[subChannel], x);

    for (int i = 0; i < CompressorParamHolder::numChannels; ++i) {
        if (i != channel) {
            assertNE(c.getRatio(i), x);
        }
    }
}

static void testAttack2() {
    CompressorParamHolder c;
    float value = .883f;
    int ivalue = 52;
    for (int i = 0; i < 16; ++i) {
        c.setAttack(i, value);
        assertEQ(c.getAttack(i), value);

        c.setRelease(i, value);
        assertEQ(c.getRelease(i), value);

        c.setThreshold(i, value);
        assertEQ(c.getThreshold(i), value);

        c.setMakeupGain(i, value);
        assertEQ(c.getMakeupGain(i), value);

        c.setWetDry(i, value);
        assertEQ(c.getWetDryMix(i), value);

        c.setRatio(i, ivalue);
        assertEQ(c.getRatio(i), ivalue);

        c.setEnabled(i, true);
        assert(c.getEnabled(i));

        c.setSidechainEnabled(i, true);
        assert(c.getSidechainEnabled(i));


        assert(c.getNumParams() == 8);
    }
}

 static void testParamChannel() {
     CompressorParamHolder h;
     CompressorParamChannel ch;

     h.setAttack(5, 123);
     h.setRelease(5, 456);
     h.setEnabled(5, true);
     h.setMakeupGain(5, 678);
     h.setRatio(5, 5);
     h.setThreshold(5, 987);
     h.setWetDry(5, .1f);
     h.setSidechainEnabled(5, true);
     assert(h.getNumParams() == 8);

     assertEQ(h.getAttack(5), 123);
     assertEQ(h.getRelease(5), 456);
     assertEQ(h.getEnabled(5), true);
     assertEQ(h.getSidechainEnabled(5), true);
     assertEQ(h.getMakeupGain(5), 678);
     assertEQ(h.getRatio(5), 5);
     assertEQ(h.getThreshold(5), 987);
     assertEQ(h.getWetDryMix(5), .1f);


     ch.copyFromHolder(h, 5);
     assertEQ(ch.attack, 123);
     assertEQ(ch.release, 456);
     assertEQ(ch.enabled, true);
     assertEQ(ch.sidechainEnabled, true);
     assertEQ(ch.ratio, 5);
     assertEQ(ch.threshold, 987);
     assertEQ(ch.wetDryMix, .1f);
 }

void testCompressorParamHolder() {
    test0();
    testInitValues();
    for (int i = 0; i < CompressorParamHolder::numChannels; ++i) {
        testAttack(i);

        testRelease(i);
        testThreshold(i);
        testMakeupGain(i);
        testEnabled(i);
        testSidechainEnabled(i);
        testWetDry(i);
        testRatio(i);
    }
    testAttack2();
    testParamChannel();
}