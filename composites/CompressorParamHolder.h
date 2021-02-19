
#pragma once

#include "SimdBlocks.h"

/**
 * this holds all the compressor params for all the channels.
 * most of the getters and setters return values for four channels,
 * these are called banks.
 */
class CompressorParmHolder {
public:
    static const unsigned int numChannels = {16};
    static const unsigned int numBanks = {numChannels / 4};

    float_4 getAttacks(unsigned int bank) const;
    float_4 getReleases(unsigned int bank) const;
    float_4 getThresholds(unsigned int bank) const;
    float_4 getMakeupGain(unsigned int bank) const;
    float_4 getEnabled(unsigned int bank) const;
    float_4 wetDryMix(unsigned int bank) const;
    void getRatio(unsigned int bank, int* ratios) const;

    /**
     * setters return true if the value was changed
     */
    bool setAttacks(unsigned int bank, float_4 value);
    bool setReleases(unsigned int bank, float_4 value);
    bool setThresholds(unsigned int bank, float_4 value);
    bool setMakeupGain(unsigned int bank, float_4 value);
    bool setEnabled(unsigned int bank, float_4 value);
    bool setWetDry(unsigned int bank, float_4 value);
    bool setRatio(unsigned int bank, const int* ratios);
private:
    float_4 a[numBanks] = {0, 0, 0, 0};
    float_4 r[numBanks] = { 0, 0, 0, 0 };
};

inline  float_4 CompressorParmHolder::getAttacks(unsigned int bank) const {
    assert(bank < numBanks);
    return a[bank];
}

inline  float_4 CompressorParmHolder::getReleases(unsigned int bank) const {
    assert(bank < numBanks);
    return r[bank];
}

inline  bool CompressorParmHolder::setAttacks(unsigned int bank, float_4 value) {
    assert(bank < numBanks);
    float_4 temp = (a[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        a[bank] = value;
    }
    return ret;
}

inline  bool CompressorParmHolder::setReleases(unsigned int bank, float_4 value) {
    assert(bank < numBanks);
    float_4 temp = (r[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        r[bank] = value;
    }
    return ret;
}