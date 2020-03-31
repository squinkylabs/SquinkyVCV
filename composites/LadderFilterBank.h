#pragma once
#include "LadderFilter.h"

#include "SqPort.h"

template <typename T>
class LadderFilterBank
{
public:
   // using Type = typename LadderFilter<T>::Types;
    void stepn(float sampleTime, int numChannels,
        SqInput& fc1Input, SqInput& fc2Input, SqInput& qInput, SqInput& driveInput, SqInput& edgeInput, SqInput& slopeInput,
            float fcParam, float fc1TrimParam, float fc2TrimParam,
            T volume,
            float qParam, float qTrimParam, float makeupGainParam,
            typename LadderFilter<T>::Types type,  typename LadderFilter<T>::Voicing voicing,
            float driveParam, float driveTrim,
            float edgeParam, float edgeTrim,
            float slopeParam, float slopeTrim,
            float spread);
           
    void step(int numChannels, bool stereoMode,
         SqInput& audioInput,  SqOutput& audioOutput);
private:
    LadderFilter<T> filters[16];

    std::shared_ptr<LookupTableParams<T>> expLookup = ObjectCache<T>::getExp2();            // Do we need more precision?
    AudioMath::ScaleFun<float> scaleGain = AudioMath::makeLinearScaler<float>(0, 1);
    std::shared_ptr<LookupTableParams<float>> audioTaper = {ObjectCache<float>::getAudioTaper()};

    AudioMath::ScaleFun<float> scaleFc = AudioMath::makeScalerWithBipolarAudioTrim(-5, 5);
    AudioMath::ScaleFun<float> scaleQ = AudioMath::makeScalerWithBipolarAudioTrim(0, 4);
    AudioMath::ScaleFun<float> scaleSlope = AudioMath::makeScalerWithBipolarAudioTrim(0, 3);
    AudioMath::ScaleFun<float> scaleEdge = AudioMath::makeScalerWithBipolarAudioTrim(0, 1);
};

template <typename T>
void LadderFilterBank<T>::stepn(float sampleTime, int numChannels,
    SqInput& fc1Input, SqInput& fc2Input, SqInput& qInput, SqInput& driveInput, SqInput& edgeInput, SqInput& slopeInput,
    float fcParam, float fc1TrimParam, float fc2TrimParam,
    T volume,
    float qParam, float qTrimParam, float makeupGainParam,
    typename LadderFilter<T>::Types type,  typename LadderFilter<T>::Voicing voicing,
    float driveParam, float driveTrimParam,
    float edgeParam, float edgeTrim,
    float slopeParam, float slopeTrim,
    float spreadParam)
{
    for (int channel=0; channel < numChannels; ++channel) {
        LadderFilter<T>& filt = filters[channel];
        filt.setType(type);
        filt.setVoicing(voicing);
        // filter Fc calc
        {
            T fcClipped = 0;
            T freqCV1 = scaleFc(
                fc1Input.getPolyVoltage(channel),
                fcParam,
                fc1TrimParam);
            T freqCV2 = scaleFc(
                fc2Input.getPolyVoltage(channel),
                0,
                fc2TrimParam);          // note: test second inputs
            T freqCV = freqCV1 + freqCV2 + 6;
            const T fc = LookupTable<T>::lookup(*expLookup, freqCV, true) * 10;
            const T normFc = fc * sampleTime;

            fcClipped = std::min(normFc, T(.48));
            fcClipped = std::max(fcClipped, T(.0000001));
            filt.setNormalizedFc(fcClipped);
        }

        // q and makeup gain
        {
            T res = scaleQ(
                qInput.getPolyVoltage(0),
                qParam,
                qTrimParam);
            const T qMiddle = 2.8;
            res = (res < 2) ?
                (res * qMiddle / 2) :
                .5 * (res - 2) * (4 - qMiddle) + qMiddle;

            if (res < 0 || res > 4) fprintf(stderr, "res out of bounds %f\n", res);

            T bAmt = makeupGainParam;
            T makeupGain = 1;
            makeupGain = 1 + bAmt * (res);

            filt.setFeedback(res);
            filt.setBassMakeupGain(makeupGain);

        }

        // gain
        {
            float  gainInput = scaleGain(
                driveInput.getPolyVoltage(channel),
                driveParam,
                driveTrimParam);

            T gain = T(.15) + 4 * LookupTable<float>::lookup(*audioTaper, gainInput, false);
            filt.setGain(gain);
        }
        {
            const float edge = scaleEdge(
                edgeInput.getVoltage(channel),
                edgeParam,
                edgeTrim);
            filt.setEdge(edge);

        }
        {  
            T slope = scaleSlope(
                slopeInput.getPolyVoltage(channel),
                slopeParam,
                slopeTrim);

        }
        filt.setFreqSpread(spreadParam);

       
    }
    
}

template <typename T>
void LadderFilterBank<T>::step(int numChannels, bool stereoMode,
         SqInput& audioInput,  SqOutput& audioOutput)
{
    assert(!stereoMode);
    for (int channel=0; channel < numChannels; ++channel) {
        LadderFilter<T>& filt = filters[channel];

        const float input =audioInput.getVoltage(channel);
            filt.run(input);
            const float output = (float) filt.getOutput();
            audioOutput.setVoltage(output, channel);
    }
}