
#pragma once

#include "GateTrigger.h"
#include "IIRDecimator.h"
#include "NonUniformLookupTable.h"
#include "SqPort.h"
#include "StateVariable4PHP.h"

class SawtoothDetuneCurve {
public:
    /**
    * @param depth is "detune" knob. 0..1
    * returns a number such that freq = detuneFactor * initialFreq
    */
    float getDetuneFactor(float depth) const {
        return NonUniformLookupTable<float>::lookup(table, depth);
    }

    SawtoothDetuneCurve() {
        // this data is pretty regular - could use uniform table
        using T = NonUniformLookupTable<float>;
        T::addPoint(table, 0, 0);
        T::addPoint(table, .0551f, .00967f);
        T::addPoint(table, .118f, .022f);
        T::addPoint(table, .181f, .04f);
        T::addPoint(table, .244f, .0467f);
        T::addPoint(table, .307f, .059f);

        T::addPoint(table, .37f, .0714f);
        T::addPoint(table, .433f, .0838f);
        T::addPoint(table, .496f, .0967f);
        T::addPoint(table, .559f, .121f);
        T::addPoint(table, .622f, .147f);
        T::addPoint(table, .748f, .243f);
        T::addPoint(table, .811f, .293f);
        T::addPoint(table, .874f, .343f);
        T::addPoint(table, .937f, .392f);
        T::addPoint(table, 1, 1);
        NonUniformLookupTable<float>::finalize(table);
    }

private:
    NonUniformLookupTableParams<float> table;
};

class SuperDpsCommonData {
public:
    // TODO: we don't need 16 of these!!!
    // this could all be global
    std::function<float(float)> expLookup = ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaper = ObjectCache<float>::getAudioTaper();
    AudioMath::ScaleFun<float> scaleDetune = AudioMath::makeLinearScaler<float>(0, 1);
    SawtoothDetuneCurve detuneCurve;
    AudioMath::RandomUniformFunc random = AudioMath::random();
};

/**
 * the signal processing for one channel
 * of saws
 */
class SuperDsp {
public:
    SuperDsp();
    /**
     * divisor is 4 for 4X oversampling, etc.
     */
    void setupDecimationRatio(int divisor);

    void step(const SuperDpsCommonData&,
              int channel, int oversampleRate, bool isStereo, float* bufferLeft, float* bufferRight,
              SqOutput& leftOut, SqOutput& rightOut, SqInput& triggerInput);

    void updatePhaseInc(const SuperDpsCommonData&,
                        int channel, int oversampleRate, float sampleTime, SqInput& cvInput,
                        float fineTuneParam, float semiParam, float octaveParam, SqInput& fmInput,
                        float fmParam, SqInput& detuneInput, float detuneParam, float detuneTrimParam);
    void updateHPFilters(bool isStereo);
    void updateMix(const SuperDpsCommonData& data, int channel, SqInput& mixInput, float mixParam, float mixTrimParam);
    void updateStereoGains(bool hardPan);

    int _stepCalls = 0;
    int _updatePhaseIncCalls = 0;

private:
    IIRDecimator<float> decimatorLeft;
    IIRDecimator<float> decimatorRight;
    StateVariable4PHP hpfLeft;
    StateVariable4PHP hpfRight;
    GateTrigger gateTrigger;

    static const int numSaws = 7;
    float globalPhaseInc = 0;
    float phase[numSaws] = {0};
    float phaseInc[numSaws] = {0};

    float gainCenter = 0;
    float gainSides = 0;

    // current left and right gains
    float sawGainsStereo[2][numSaws] = {
        {.2f},
        {.2f}};

    float sawGainsNorm[2][numSaws] = {
        {1.f, .26f, .87f, .71f, .5f, .97f, 0.f},
        {0.f, .97f, .5f, .71f, .87f, .26f, 1.f}};

    float sawGainsHardPan[2][numSaws] = {
        {1.1f, 0.f, 1.1f, 1.f, 0.f, 1.1f, 0.f},
        {0.f, 1.1f, 0.f, 1.f, 1.1f, 0.f, 1.1f}};

    // TODO: make static
    float const detuneFactors[numSaws] = {
        .89f,
        .94f,
        .98f,
        1.f,
        1.02f,
        1.06f,
        1.107f};

    void updateAudioClassic(int channel, SqOutput& leftOut, SqOutput& rightOut);
    void updateTrigger(const SuperDpsCommonData& data, SqInput& triggerInput, int channel);
    void updateAudioClean(int channel, float* buffer, SqOutput& leftOut, SqOutput& rightOut, int oversampleRate);
    void updateAudioClassicStereo(int channel, SqOutput& leftOut, SqOutput& rightOut);
    void updateAudioCleanStereo(int channel, float* bufferLeft, float* bufferRight, SqOutput& leftOut, SqOutput& rightOut, int oversampleRate);
    void runSaws(float& left);
    void runSawsStereo(float& left, float& right);
};

inline SuperDsp::SuperDsp() : gateTrigger(true) {
}

// is this function even necessary? I think it's just initial setup
inline void SuperDsp::setupDecimationRatio(int decimateDiv) {
    decimatorLeft.setup(decimateDiv);
    decimatorRight.setup(decimateDiv);
}

inline void SuperDsp::step(const SuperDpsCommonData& data,
                           int channel, int oversampleRate, bool isStereo, float* bufferLeft, float* bufferRight,
                           SqOutput& leftOut, SqOutput& rightOut,
                           Input& triggerInput) {
    ++_stepCalls;
    if ((oversampleRate == 1) && !isStereo) {
        updateAudioClassic(channel, leftOut, rightOut);
    } else if ((oversampleRate == 1) && isStereo) {
        updateAudioClassicStereo(channel, leftOut, rightOut);
    } else if ((oversampleRate != 1) && !isStereo) {
        updateAudioClean(channel, bufferLeft, leftOut, rightOut, oversampleRate);
    } else {
        updateAudioCleanStereo(channel, bufferLeft, bufferRight, leftOut, rightOut, oversampleRate);
    }
    updateTrigger(data, triggerInput, channel);
}

inline void SuperDsp::updateAudioClassic(int channel, SqOutput& leftOut, SqOutput& rightOut) {
    float left;
    runSaws(left);

    const float output = hpfLeft.run(left);
    leftOut.setVoltage(output, channel);
    rightOut.setVoltage(output, channel);
}

inline void SuperDsp::updateAudioClean(int channel, float* buffer, SqOutput& leftOut, SqOutput& rightOut, int oversampleRate) {
    const int bufferSize = oversampleRate;
    decimatorLeft.setup(bufferSize);
    for (int i = 0; i < bufferSize; ++i) {
        float left;
        runSaws(left);
        buffer[i] = left;
    }

    const float output = decimatorLeft.process(buffer);
    leftOut.setVoltage(output, channel);
    rightOut.setVoltage(output, channel);
}

inline void SuperDsp::updateAudioClassicStereo(int channel, SqOutput& leftOut, SqOutput& rightOut) {
    float left, right;
    runSawsStereo(left, right);

    const float outputLeft = hpfLeft.run(left);
    const float outputRight = hpfRight.run(right);
    leftOut.setVoltage(outputLeft, channel);
    rightOut.setVoltage(outputRight, channel);
}

inline void SuperDsp::updateAudioCleanStereo(int channel, float* bufferLeft, float* bufferRight, SqOutput& leftOut, SqOutput& rightOut, int oversampleRate) {
    const int bufferSize = oversampleRate;
    decimatorLeft.setup(bufferSize);
    decimatorRight.setup(bufferSize);
    for (int i = 0; i < bufferSize; ++i) {
        float left, right;
        runSawsStereo(left, right);
        bufferLeft[i] = left;
        bufferRight[i] = right;
    }

    const float outputLeft = decimatorLeft.process(bufferLeft);
    const float outputRight = decimatorRight.process(bufferRight);
    leftOut.setVoltage(outputLeft, channel);
    rightOut.setVoltage(outputRight, channel);
}

inline void SuperDsp::runSaws(float& left) {
    float mix = 0;
    for (int i = 0; i < numSaws; ++i) {
        phase[i] += phaseInc[i];
        if (phase[i] > 1) {
            phase[i] -= 1;
        }
        assert(phase[i] <= 1);
        assert(phase[i] >= 0);

        const float gain = (i == numSaws / 2) ? gainCenter : gainSides;
        mix += (phase[i] - .5f) * gain;  // experiment to get rid of DC
    }

    mix *= 4.5;  // too low 2 too high 10
    left = mix;
}

inline void SuperDsp::runSawsStereo(float& left, float& right) {
    left = right = 0;
    for (int i = 0; i < numSaws; ++i) {
        phase[i] += phaseInc[i];
        if (phase[i] > 1) {
            phase[i] -= 1;
        }
        assert(phase[i] <= 1);
        assert(phase[i] >= 0);

        left += (phase[i] - .5f) * sawGainsStereo[0][i];
        right += (phase[i] - .5f) * sawGainsStereo[1][i];
    }
}

inline void SuperDsp::updatePhaseInc(const SuperDpsCommonData& data,
                                     int channel, int oversampleRate, float sampleTime, SqInput& cvInput,
                                     float fineTuneParam, float semiParam, float octaveParam, SqInput& fmInput,
                                     float fmParam, SqInput& detuneCVInput, float detuneParam, float detuneTrimParam) {
    ++_updatePhaseIncCalls;
    const float finePitch = fineTuneParam / 12.0f;
    const float semiPitch = semiParam / 12.0f;

    float pitch = 1.0f + roundf(octaveParam) +
                  semiPitch +
                  finePitch;

    pitch += cvInput.getPolyVoltage(channel);

    const float fm = fmInput.getPolyVoltage(channel);

    const float fmDepth = AudioMath::quadraticBipolar(fmParam);
    // printf("fm cv = %.2f depth=%.2f\n", fm, fmDepth);
    // printf("address of fm input in dsp =%p\n", &fmInput);
    pitch += (fmDepth * fm);
    // printf("final pitch[%d] = %.2f\n", channel, pitch);

    const float q = float(log2(261.626));  // move up to pitch range of EvenVCO
    pitch += q;
    const float freq = data.expLookup(pitch);
    // printf("final pitch[%d] = %.2f freq=%.2f\n", channel, pitch, freq);
    globalPhaseInc = sampleTime * freq;
    assert(sampleTime < .01);
    assert(globalPhaseInc > 0 && globalPhaseInc < .4);  // just for debuggin

    const float rawDetuneValue = data.scaleDetune(
        detuneCVInput.getPolyVoltage(channel),
        detuneParam,
        detuneTrimParam);

    const float detuneInput = data.detuneCurve.getDetuneFactor(rawDetuneValue);
#if 0
    printf("detuneCV = %.2f param =%.2f trim=%.2f final=%.2f\n",
        detuneCVInput.getPolyVoltage(channel),
        detuneParam,
        detuneTrimParam,
        detuneInput);
#endif

    for (int i = 0; i < numSaws; ++i) {
        float detune = (detuneFactors[i] - 1) * detuneInput;

        detune += 1;
        float phaseIncI = globalPhaseInc * detune;

        assert(globalPhaseInc > 0 && globalPhaseInc < .4);  // just for debuggin

        phaseIncI = std::min(phaseIncI, .4f);  // limit so saws don't go crazy
        if (oversampleRate > 1) {
            phaseIncI /= oversampleRate;
        }
        assert(phaseIncI > 0 && phaseIncI < .1);
        phaseInc[i] = phaseIncI;
    }
}

inline void SuperDsp::updateHPFilters(bool isStereo) {
    const float filterCutoff = std::min(globalPhaseInc, .1f);
    hpfLeft.setCutoff(filterCutoff);
    if (isStereo) {
        hpfRight.setCutoff(filterCutoff);
    }
}

inline void SuperDsp::updateTrigger(const SuperDpsCommonData& data, Input& input, int channel) {
    const float triggerInput = input.getPolyVoltage(channel);
    gateTrigger.go(triggerInput);
    if (gateTrigger.trigger()) {
        for (int i = 0; i < numSaws; ++i) {
            phase[i] = data.random();
        }
    }
}

inline void SuperDsp::updateMix(const SuperDpsCommonData& data, int channel, SqInput& mixInput, float mixParam, float mixTrimParam) {
    const float rawMixValue = data.scaleDetune(
        mixInput.getPolyVoltage(channel),
        mixParam,
        mixTrimParam);

    gainCenter = -0.55366f * rawMixValue + 0.99785f;

    gainSides = -0.73764f * rawMixValue * rawMixValue +
                1.2841f * rawMixValue + 0.044372f;
}

inline void SuperDsp::updateStereoGains(bool hardPan) {
    for (int i = 0; i < numSaws; ++i) {
        const float monoGain = 4.5f * ((i == numSaws / 2) ? gainCenter : gainSides);

        float l = monoGain;
        float r = monoGain;

        if (!hardPan) {
            l *= sawGainsNorm[0][i];
            r *= sawGainsNorm[1][i];
        } else {
            l *= sawGainsHardPan[0][i];
            r *= sawGainsHardPan[1][i];
        }

        sawGainsStereo[0][i] = l;
        sawGainsStereo[1][i] = r;
    }
}

// The processing params that can be
// shared between all dsp blocks
class SuperDspCommon : public SuperDpsCommonData {
public:
    static const unsigned int MAX_OVERSAMPLE = 16;
    /**
     * divisor is 4 for 4X oversampling, etc.
     */
    void setupDecimationRatio(int divisor, int numChannels);

    /**
     * called every sample to calc audio.
     */
    void step(int numChannels, bool isStereo, SqOutput& leftOut, SqOutput& rightOut,
              int oversampleRate, SqInput& triggerInput);

    /**
     * called every 'n' sample to calc CV.
     * must be called for each active channel.
     */
    void stepn(int n, int index, int oversampleRate, float sampleTime, SqInput& cvInput,
               float fineTuneParam, float semiParam, float octaveParam, SqInput& fmInput,
               float fmParam, SqInput& detuneInput, float detuneParam, float detuneTrimParam,
               SqInput& mixInput, float mixParam, float mixTrimPara,
               bool isStereo,
               bool hardPan);

    SuperDsp& _getDsp(unsigned n) {
        assert(n < 16);
        return dsp[n];
    }

private:
    float bufferLeft[MAX_OVERSAMPLE] = {0};
    float bufferRight[MAX_OVERSAMPLE] = {0};

    SuperDsp dsp[16];  // maximum 16 channels

    std::function<float(float)> expLookup =
        ObjectCache<float>::getExp2Ex();
    std::shared_ptr<LookupTableParams<float>> audioTaper =
        ObjectCache<float>::getAudioTaper();
};

inline void SuperDspCommon::setupDecimationRatio(int divisor, int numChannels) {
    for (int i = 0; i < numChannels; ++i) {
        dsp[i].setupDecimationRatio(divisor);
    }
}

inline void SuperDspCommon::step(int numChannels, bool isStereo, SqOutput& leftOut, SqOutput& rightOut,
                                 int oversampleRate, SqInput& triggerInput) {
    for (int i = 0; i < numChannels; ++i) {
        dsp[i].step(*this, i, oversampleRate, isStereo, bufferLeft, bufferRight, leftOut, rightOut, triggerInput);
    }
}

inline void SuperDspCommon::stepn(int n, int channel, int oversampleRate, float sampleTime, SqInput& cvInput,
                                  float fineTuneParam, float semiParam, float octaveParam, SqInput& fmInput,
                                  float fmParam, SqInput& detuneInput, float detuneParam, float detuneTrimParam,
                                  SqInput& mixInput, float mixParam, float mixTrimParam,
                                  bool isStereo,
                                  bool hardPan) {
    SuperDsp& d = dsp[channel];

    d.updatePhaseInc(*this, channel, oversampleRate, sampleTime, cvInput, fineTuneParam, semiParam, octaveParam, fmInput, fmParam, detuneInput, detuneParam, detuneTrimParam);
    d.updateHPFilters(isStereo);
    d.updateMix(*this, channel, mixInput, mixParam, mixTrimParam);
    d.updateStereoGains(hardPan);
}
