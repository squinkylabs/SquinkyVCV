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
};