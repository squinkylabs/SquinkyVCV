#pragma once

#include <memory>

class FractionalDelay
{
public:
    FractionalDelay(int numSamples) : numSamples(numSamples), delayMemory( new float[numSamples])
    {
        for (int i = 0; i < numSamples; ++i) {
            delayMemory[i] = 0;
        }
    }
    ~FractionalDelay()
    {
        delete delayMemory;
    }

    void setDelay(float samples)
    {

    }
    float run(float input)
    {
        float ret = getOutput();
        setInput(input);
        return ret;
    }
private:
  
    /**
     * get the fractional delayed output, based in delayTime
     */
    float getOutput();

    /**
     * send the next input to the delay line
     */
    void setInput(float);

    /**
     * get delay output with integer (non-fractional) delay time
     */
    float getDelayedOutput(int delaySamples);

    double delayTime = 0;
    int inputPointerIndex = 0;

    /**
     * The size of the delay line, in samples
     */
    const int numSamples;

    float* delayMemory;

};


