#pragma once
#include "LadderFilter.h"

#include "SqPort.h"

template <typename T>
class LadderFilterBank
{
public:
    void stepn(int numChannels);
    void step(int numChannels, bool stereoMode,
         SqInput& audioInput,  SqOutput& audioOutput);
private:
    LadderFilter<T> filters[16];
};

template <typename T>
void LadderFilterBank<T>::stepn(int numChannels)
{

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