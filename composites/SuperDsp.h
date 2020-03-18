
#pragma once




/**
 * the signal processing for one channel
 * of saws
 */
class SuperDsp
{
public:
    /**
     * divisor is 4 for 4X oversampling, etc.
     */
    void setupDecimationRatio(int divisor);
private:
  
};

inline void SuperDsp:: setupDecimationRatio(int divisor)
{
    assert(false);
}

// The processing params that can be 
// shared between all dsp blocks
class SuperDspCommon
{
public:

    static const unsigned int MAX_OVERSAMPLE = 16;
     /**
     * divisor is 4 for 4X oversampling, etc.
     */
    void setupDecimationRatio(int divisor);

    /**
     * called every sample to calc audio.
     */
    void step();

     /**
     * called every 'nn' sample to calc CV.
     */
    void stepn(int n);
private:
    float bufferLeft[MAX_OVERSAMPLE] = {0};
    float bufferRight[MAX_OVERSAMPLE] = {0};

     SuperDsp dsp[16];   // maximum 16 channels
     int numChannels = 1;
};

inline void SuperDspCommon::setupDecimationRatio(int divisor)
{
    for (int i=0; i<numChannels; ++i) {
        dsp[i].setupDecimationRatio(divisor);
    }
}

inline  void SuperDspCommon::step()
{
 assert(false);
}

inline  void SuperDspCommon::stepn(int n)
{
 assert(false);
}
