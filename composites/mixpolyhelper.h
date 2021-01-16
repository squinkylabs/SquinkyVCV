#pragma once

template <class TMixComposite>
class MixPolyHelper {
public:
    MixPolyHelper() {
    }

    void updatePolyphony(TMixComposite*);
    float getNormalizedInputSum(TMixComposite*, int channel);

private:
    float gain[TMixComposite::numChannels] = {0};
};

template <class TMixComposite>
inline void MixPolyHelper<TMixComposite>::updatePolyphony(TMixComposite* mixer) {
    for (int i = 0; i < TMixComposite::numChannels; ++i) {
        const int channels = mixer->inputs[TMixComposite::AUDIO0_INPUT + i].getChannels();
        assert(channels >= 0 && channels <= 16);
        gain[i] = channels == 0 ? 0 : 1.f / channels;
    }
}

template <class TMixComposite>
inline float MixPolyHelper<TMixComposite>::getNormalizedInputSum(TMixComposite* mixer, int channel) {
    const float sum = mixer->inputs[TMixComposite::AUDIO0_INPUT + channel].getVoltageSum();
    return sum * gain[channel];
}