
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

    void setAttack(unsigned int channel, float value);
    void setRelease(unsigned int channel, float value);
    void setThreshold(unsigned int channel, float value);
    void setMakeupGain(unsigned int channel, float value);
    void setEnabled(unsigned int channel, bool value);
    void setWetDry(unsigned int channel, float value);
    void setRatio(unsigned int channel, int ratios);

private:
    float_4 a[numBanks] = {0, 0, 0, 0};
    float_4 r[numBanks] = {0, 0, 0, 0};
    float_4 t[numBanks] = {0, 0, 0, 0};
    float_4 m[numBanks] = {0, 0, 0, 0};
    float_4 e[numBanks] = {0, 0, 0, 0};

    float_4 w[numBanks] = {0, 0, 0, 0};
    int32_4 ratio[numBanks] = {0, 0, 0, 0};
};

inline float_4 CompressorParmHolder::getAttacks(unsigned int bank) const {
    assert(bank < numBanks);
    return a[bank];
}

inline float CompressorParmHolder::getAttack(unsigned int channel) const {
    assert(channel < numChannels);
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return a[bank][subChannel];
}

inline float_4 CompressorParmHolder::getReleases(unsigned int bank) const {
    assert(bank < numBanks);
    return r[bank];
}

inline float CompressorParmHolder::getRelease(unsigned int channel) const {
    assert(channel < numChannels);
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return r[bank][subChannel];
}

inline float_4 CompressorParmHolder::getThresholds(unsigned int bank) const {
    assert(bank < numBanks);
    return t[bank];
}

inline float CompressorParmHolder::getThreshold(unsigned int channel) const {
    assert(channel < numChannels);
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return t[bank][subChannel];
}

inline float_4 CompressorParmHolder::getMakeupGains(unsigned int bank) const {
    assert(bank < numBanks);
    return m[bank];
}

inline float CompressorParmHolder::getMakeupGain(unsigned int channel) const {
    assert(channel < numChannels);
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return m[bank][subChannel];
}

inline float_4 CompressorParmHolder::getEnableds(unsigned int bank) const {
    assert(bank < numBanks);
    return e[bank];
}

inline bool CompressorParmHolder::getEnabled(unsigned int channel) const {
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return e[bank][subChannel] != 0.f;
}

inline float_4 CompressorParmHolder::getWetDryMixs(unsigned int bank) const {
    assert(bank < numBanks);
    return w[bank];
}

inline float CompressorParmHolder::getWetDryMix(unsigned int channel) const {
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return w[bank][subChannel];
}

inline int CompressorParmHolder::getRatio(unsigned int channel) const {
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return ratio[bank][subChannel];
}

inline int32_4 CompressorParmHolder::getRatios(unsigned int bank) const {
    assert(bank < numBanks);
    return ratio[bank];
}

inline void CompressorParmHolder::setAttack(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    a[bank][subChannel] = value;
}

inline void CompressorParmHolder::setRelease(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    r[bank][subChannel] = value;
}

inline void CompressorParmHolder::setThreshold(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    t[bank][subChannel] = value;
}

inline void CompressorParmHolder::setMakeupGain(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    m[bank][subChannel] = value;
}

inline void CompressorParmHolder::setEnabled(unsigned int channel, bool value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    e[bank][subChannel] = value ? SimdBlocks::maskTrue()[0] : SimdBlocks::maskFalse()[0];
}

inline void CompressorParmHolder::setWetDry(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    w[bank][subChannel] = value;
}

inline void CompressorParmHolder::setRatio(unsigned int channel, int value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    ratio[bank][subChannel] = value;
}