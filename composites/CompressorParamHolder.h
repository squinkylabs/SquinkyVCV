
#pragma once

#include "SimdBlocks.h"

/**
 * This holds all the compressor params for all the channels.
 * many of the getters and setters return values for four channels,
 * these are called banks.
 */
class CompressorParamHolder {
public:
    CompressorParamHolder() = default;
    CompressorParamHolder(const CompressorParamHolder&) = delete;
    const CompressorParamHolder& operator=(const CompressorParamHolder&) = delete;
    static const unsigned int numChannels = {16};
    static const unsigned int numBanks = {numChannels / 4};

    void copy(unsigned int dest, unsigned int src);

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

    float_4 getSidechainEnableds(unsigned int bank) const;
    bool getSidechainEnabled(unsigned int channel) const;

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
    void setSidechainEnabled(unsigned int channel, bool value);

    static int getNumParams() { return 8; }

private:
    float_4 a[numBanks] = {0, 0, 0, 0};
    float_4 r[numBanks] = {0, 0, 0, 0};
    float_4 t[numBanks] = {0, 0, 0, 0};
    float_4 m[numBanks] = {0, 0, 0, 0};
    float_4 e[numBanks] = {0, 0, 0, 0};
    float_4 se[numBanks] = { 0, 0, 0, 0 };

    float_4 w[numBanks] = {0, 0, 0, 0};
    int32_4 ratio[numBanks] = {0, 0, 0, 0};
};

class CompressorParamChannel {
public:
    float attack = 0;
    float release = 0;
    float threshold = 0;
    float makeupGain = 0;
    bool enabled = false;
    float wetDryMix = 0;
    int ratio = 0;
    bool sidechainEnabled = false;

    /** copies data from CompressorParamHolder to us
     */
    void copyFromHolder(const CompressorParamHolder&, int channel);
   // void copyToHolder(CompressorParamHolder&, int channel);
};

inline void CompressorParamChannel::copyFromHolder(const CompressorParamHolder& h, int channel) {
    attack = h.getAttack(channel);
    release = h.getRelease(channel);
    threshold = h.getThreshold(channel);
    makeupGain = h.getMakeupGain(channel);
    enabled = h.getEnabled(channel);
    sidechainEnabled = h.getSidechainEnabled(channel);
    wetDryMix = h.getWetDryMix(channel);
    ratio = h.getRatio(channel);
     assert(CompressorParamHolder::getNumParams() == 8);
}

inline void CompressorParamHolder::copy(unsigned int dest, unsigned int src) {
    assert(getNumParams() == 8);
    assert(dest < numChannels);
    assert(dest < 16);
    assert(src < 16);
    const unsigned destBank = dest / 4;
    const unsigned destSubChannel = dest - (destBank * 4);

    const unsigned srcBank = src / 4;
    const unsigned srcSubChannel = src - (srcBank * 4);

    a[destBank][destSubChannel] = a[srcBank][srcSubChannel];
    r[destBank][destSubChannel] = r[srcBank][srcSubChannel];
    t[destBank][destSubChannel] = t[srcBank][srcSubChannel];
    m[destBank][destSubChannel] = m[srcBank][srcSubChannel];
    e[destBank][destSubChannel] = e[srcBank][srcSubChannel];
    se[destBank][destSubChannel] = se[srcBank][srcSubChannel];
    w[destBank][destSubChannel] = w[srcBank][srcSubChannel];
    ratio[destBank][destSubChannel] = ratio[srcBank][srcSubChannel];
}

inline float_4 CompressorParamHolder::getAttacks(unsigned int bank) const {
    assert(bank < numBanks);
    return a[bank];
}

inline float CompressorParamHolder::getAttack(unsigned int channel) const {
    assert(channel < numChannels);
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return a[bank][subChannel];
}

inline float_4 CompressorParamHolder::getReleases(unsigned int bank) const {
    assert(bank < numBanks);
    return r[bank];
}

inline float CompressorParamHolder::getRelease(unsigned int channel) const {
    assert(channel < numChannels);
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return r[bank][subChannel];
}

inline float_4 CompressorParamHolder::getThresholds(unsigned int bank) const {
    assert(bank < numBanks);
    return t[bank];
}

inline float CompressorParamHolder::getThreshold(unsigned int channel) const {
    assert(channel < numChannels);
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return t[bank][subChannel];
}

inline float_4 CompressorParamHolder::getMakeupGains(unsigned int bank) const {
    assert(bank < numBanks);
    return m[bank];
}

inline float CompressorParamHolder::getMakeupGain(unsigned int channel) const {
    assert(channel < numChannels);
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return m[bank][subChannel];
}

inline float_4 CompressorParamHolder::getEnableds(unsigned int bank) const {
    assert(bank < numBanks);
    return e[bank];
}

inline float_4 CompressorParamHolder::getSidechainEnableds(unsigned int bank) const {
    assert(bank < numBanks);
    return se[bank];
}

inline bool CompressorParamHolder::getEnabled(unsigned int channel) const {
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return e[bank][subChannel] != 0.f;
}

inline bool CompressorParamHolder::getSidechainEnabled(unsigned int channel) const {
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return se[bank][subChannel] != 0.f;
}

inline float_4 CompressorParamHolder::getWetDryMixs(unsigned int bank) const {
    assert(bank < numBanks);
    return w[bank];
}

inline float CompressorParamHolder::getWetDryMix(unsigned int channel) const {
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return w[bank][subChannel];
}

inline int CompressorParamHolder::getRatio(unsigned int channel) const {
    const unsigned bank = channel / 4;
    const unsigned subChannel = channel - (bank * 4);
    return ratio[bank][subChannel];
}

inline int32_4 CompressorParamHolder::getRatios(unsigned int bank) const {
    assert(bank < numBanks);
    return ratio[bank];
}

inline void CompressorParamHolder::setAttack(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    a[bank][subChannel] = value;
}

inline void CompressorParamHolder::setRelease(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    r[bank][subChannel] = value;
}

inline void CompressorParamHolder::setThreshold(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    t[bank][subChannel] = value;
}

inline void CompressorParamHolder::setMakeupGain(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    m[bank][subChannel] = value;
}

inline void CompressorParamHolder::setEnabled(unsigned int channel, bool value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    e[bank][subChannel] = value ? SimdBlocks::maskTrue()[0] : SimdBlocks::maskFalse()[0];
}

inline void CompressorParamHolder::setSidechainEnabled(unsigned int channel, bool value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    se[bank][subChannel] = value ? SimdBlocks::maskTrue()[0] : SimdBlocks::maskFalse()[0];
}

inline void CompressorParamHolder::setWetDry(unsigned int channel, float value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    w[bank][subChannel] = value;
}

inline void CompressorParamHolder::setRatio(unsigned int channel, int value) {
    assert(channel < numChannels);
    const unsigned int bank = channel / 4;
    const unsigned int subChannel = channel % 4;
    ratio[bank][subChannel] = value;
}