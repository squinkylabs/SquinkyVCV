#pragma once

class FFTDataCpx;
class FFTDataReal;

class FFT
{
public:
    /** Forward FFT will do the 1/N scaling
     */
    static bool forward(FFTDataCpx* out, const FFTDataReal& in);
    static bool inverse(FFTDataReal* out, const FFTDataCpx& in);

  //  static FFTDataCpx* makeNoiseFormula(float slope, float highFreqCorner, int frameSize);

    /**
     * Fills a complex FFT frame with frequency domain data describing noise
     */
    static void makeNoiseFormula(FFTDataCpx* output, float slope, float highFreqCorner, float sampleRate);
};