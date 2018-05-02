#pragma once

class FFTDataCpx;
class FFTDataReal;


class ColoredNoiseSpec
{
public:
    float slope = 0;
    float highFreqCorner = 4000;
    float sampleRate = 44100;
    bool operator != (const ColoredNoiseSpec&) const;
};

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
    static void makeNoiseSpectrum(FFTDataCpx* output, const ColoredNoiseSpec&);

    static void normalize(FFTDataReal*);
};