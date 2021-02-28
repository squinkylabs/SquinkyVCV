
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
    float_4 getMakeupGains(unsigned int bank) const;
    float_4 getEnableds(unsigned int bank) const;
    float_4 getWetDryMixs(unsigned int bank) const;
   // void getRatio(unsigned int bank, int* ratios) const;
    int32_4 getRatios(unsigned int bank) const;

    /**
     * setters return true if the value was changed
     */
    bool setAttacks(unsigned int bank, float_4 value);
    bool setReleases(unsigned int bank, float_4 value);
    bool setThresholds(unsigned int bank, float_4 value);
    bool setMakeupGains(unsigned int bank, float_4 value);
    bool setEnableds(unsigned int bank, float_4 value);
    bool setWetDrys(unsigned int bank, float_4 value);
    bool setRatios(unsigned int bank, int32_4 ratios);
private:
    float_4 a[numBanks] = {0, 0, 0, 0};
    float_4 r[numBanks] = { 0, 0, 0, 0};
    float_4 t[numBanks] = { 0, 0, 0, 0 };
    float_4 m[numBanks] = { 0, 0, 0, 0 };
    float_4 e[numBanks] = { 0, 0, 0, 0 };

    float_4 w[numBanks] = { 0, 0, 0, 0 };
    int32_4 ratio[numBanks] = { 0, 0, 0, 0 };
};

inline  float_4 CompressorParmHolder::getAttacks(unsigned int bank) const {
    assert(bank < numBanks);
    return a[bank];
}

inline  float_4 CompressorParmHolder::getReleases(unsigned int bank) const {
    assert(bank < numBanks);
    return r[bank];
}

inline  float_4 CompressorParmHolder::getThresholds(unsigned int bank) const {
    assert(bank < numBanks);
    return t[bank];
}
inline  float_4 CompressorParmHolder::getMakeupGains(unsigned int bank) const {
    assert(bank < numBanks);
    return m[bank];
}
inline  float_4 CompressorParmHolder::getEnableds(unsigned int bank) const {
    assert(bank < numBanks);
    return e[bank];
}
inline  float_4 CompressorParmHolder::getWetDryMixs(unsigned int bank) const {
    assert(bank < numBanks);
    return w[bank];
}
inline  int32_4 CompressorParmHolder::getRatios(unsigned int bank) const {
    assert(bank < numBanks);
    return ratio[bank];
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

inline  bool CompressorParmHolder::setThresholds(unsigned int bank, float_4 value) {
    assert(bank < numBanks);
    float_4 temp = (t[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        t[bank] = value;
    }
    return ret;
}

inline  bool CompressorParmHolder::setMakeupGains(unsigned int bank, float_4 value) {
    assert(bank < numBanks);
    float_4 temp = (m[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        m[bank] = value;
    }
    return ret;
}

inline  bool CompressorParmHolder::setEnableds(unsigned int bank, float_4 value) {
    assert(bank < numBanks);
    float_4 temp = (e[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        e[bank] = value;
    }
    return ret;
}

inline  bool CompressorParmHolder::setWetDrys(unsigned int bank, float_4 value) {
    assert(bank < numBanks);
    float_4 temp = (w[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        w[bank] = value;
    }
    return ret;
}

inline  bool CompressorParmHolder::setRatios(unsigned int bank, int32_4 value) {
    assert(bank < numBanks);
    float_4 temp = (ratio[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        ratio[bank] = value;
    }
    return ret;
}