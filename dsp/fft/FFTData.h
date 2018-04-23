#pragma once

#include <complex>
#include <vector>



using cpx = std::complex<float>;

class FFTDataCpx
{
public:
    FFTDataCpx(int numBins);
    cpx get(int bin) const;
    void set(int bit, cpx value);
private:
    std::vector<cpx> buffer;
};

class FFTDataReal
{
public:
    FFTDataReal(int numBins);
    float get(int numBin) const;
    void set(int numBin, float value);
private:
    std::vector<float> buffer;
};