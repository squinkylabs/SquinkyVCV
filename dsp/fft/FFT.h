#pragma once

class FFTDataCpx;
class FFTDataReal;

class FFT
{
public:
    static bool forward(FFTDataCpx* out, const FFTDataReal& in);
    static bool inverse(FFTDataReal* out, const FFTDataCpx& in);
};