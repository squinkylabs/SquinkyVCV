
#pragma once

#include "SimdBlocks.h"

/**
 * This holds all the compressor params for all the channels.
 * many of the getters and setters return values for four channels,
 * these are called banks.
 */
class CompressorParmHolder {
public:
    static const unsigned int numChannels = {16};
    static const unsigned int numBanks = {numChannels / 4};

    float_4 getAttacks(unsigned int bank) const;
    float getAttack(unsigned int channel) const;

    float_4 getReleases(unsigned int bank) const;
    float getRelease(unsigned int channel) const;

    float_4 getThresholds(unsigned int bank) const;
    float getThreshold(unsigned int channel) const;
    
    float_4 getMakeupGains(unsigned int bank) const;
    float getMakeupGain(unsigned int channel) const;
    
    float_4 getEnableds(unsigned int bank) const;
    bool getEnabled(unsigned int channel) const;
    
    
    float_4 getWetDryMixs(unsigned int bank) const;
    float getWetDryMix(unsigned int channel) const;
   
   // void getRatio(unsigned int bank, int* ratios) const;
    int32_4 getRatios(unsigned int bank) const;
    int getRatio(unsigned int channel) const;


    /**
     * setters return true if the value was changed
     */
    bool setAttack(unsigned int channel, float value);
    bool setRelease(unsigned int channel, float value);
    bool setThreshold(unsigned int channel, float value);
    bool setMakeupGain(unsigned int channel, float value);
    bool setEnabled(unsigned int channel, bool value);
    bool setWetDry(unsigned int channel, float value);
    bool setRatio(unsigned int channel, int ratios);
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

inline  float CompressorParmHolder::getAttack(unsigned int channel) const {
    assert(channel < numChannels);
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return a[bank][subChannel];
}

inline  float_4 CompressorParmHolder::getReleases(unsigned int bank) const {
    assert(bank < numBanks);
    return r[bank];
}

inline  float CompressorParmHolder::getRelease(unsigned int channel) const {
    assert(channel < numChannels);
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return r[bank][subChannel];
}

inline  float_4 CompressorParmHolder::getThresholds(unsigned int bank) const {
    assert(bank < numBanks);
    return t[bank];
}

inline  float CompressorParmHolder::getThreshold(unsigned int channel) const {
    assert(channel < numChannels);
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return t[bank][subChannel];
}

inline  float_4 CompressorParmHolder::getMakeupGains(unsigned int bank) const {
    assert(bank < numBanks);
    return m[bank];
}

inline  float CompressorParmHolder::getMakeupGain(unsigned int channel) const {
    assert(channel < numChannels);
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return m[bank][subChannel];
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

inline  bool CompressorParmHolder::setAttack(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    float_4 temp = (a[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        a[bank] = value;
    }
    return ret;
}

inline  bool CompressorParmHolder::setRelease(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    float_4 temp = (r[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        r[bank] = value;
    }
    return ret;
}

inline  bool CompressorParmHolder::setThreshold(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    float_4 temp = (t[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        t[bank] = value;
    }
    return ret;
}

inline  bool CompressorParmHolder::setMakeupGain(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    float_4 temp = (m[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        m[bank] = value;
    }
    return ret;
}

inline  bool CompressorParmHolder::setEnabled(unsigned int channel, bool value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    float_4 temp = (e[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        e[bank] = value;
    }
    return ret;
}

inline  bool CompressorParmHolder::setWetDry(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    float_4 temp = (w[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        w[bank] = value;
    }
    return ret;
}

inline  bool CompressorParmHolder::setRatio(unsigned int channel, int value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    float_4 temp = (ratio[bank] != value);
    bool ret = SimdBlocks::isTrue(temp);
    if (ret) {
        ratio[bank] = value;
    }
    return ret;
}