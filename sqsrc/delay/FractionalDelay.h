#pragma once

#include <algorithm>
#include <assert.h>
#include <memory>

/**
 * When ignoring wrap, inputIndex > outputIndex.
 * so output "pull up the rear", reading the samples that were written
 * delayTime samples ago.
 */
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
        delete[] delayMemory;
    }

    void setDelay(float samples)
    {
        // assert(samples < numSamples);
        // leave 3 for padding for the cubic interp
        samples = std::min<float>(float(numSamples) - 3, samples);
        delayTime = samples;
    }
    float run(float input)
    {
        float ret = getOutput();
        setInput(input);
        return ret;
    }
protected:
  
    /**
     * get the fractional delayed output, based in delayTime
     */
    float getOutput();

    /**
     * send the next input to the delay line
     */
    void setInput(float);

    /**
     * The size of the delay line, in samples
     */
    const int numSamples;
private:
    /**
     * get delay output with integer (non-fractional) delay time
     */
    float getDelayedOutput(int delaySamples);

    double delayTime = 0;
    int inputPointerIndex = 0;

  

    float* delayMemory;
};

class RecirculatingFractionalDelay : public FractionalDelay
{
public:
    RecirculatingFractionalDelay(int numSamples) : FractionalDelay(numSamples)
    {
    }
#if 0
    void setDelay(float samples)
    {
        delay.setDelay(samples);
    }
#endif
    void setFeedback(float in_feedback)
    {
      //  assert(feedback < 1);
      //  assert(feedback > -1);
        feedback = in_feedback;
    }

    float run(float);

    virtual float processFeedback(float in) {
        return in;
    }
private:
  //  FractionalDelay delay;
    float feedback = 0;
};

