#pragma once

#include <complex>
#include <vector>
#include <assert.h>


class FFT;


/**
 * Our wrapper api uses std::complex, so we don't need to expose kiss_fft_cpx
 * outside. Our implementation assumes the two are equivalent, and that a
 * reinterpret_cast can bridge them.
 */
using cpx = std::complex<float>;

template <typename T>
class FFTData
{
public:
    friend FFT;
    FFTData(int numBins);
    ~FFTData();
    T get(int bin) const;
    void set(int bin, T value);

    int size() const
    {
        return (int) buffer.size();
    }

    T * data()
    {
        return buffer.data();
    }

    float getAbs(int bin) const
    {
        return std::abs(buffer[bin]);
    }

    bool isPolar() const {
        assert(false);
        return false;
    }

    void toPolar() {
        assert(false);
    }

    std::pair<float, float> getMagAndPhase(int bin) const {
        assert(false);
        return std::make_pair(0.f, 0.f);
    }

    static int _count;
private:
    std::vector<T> buffer;
    bool _isPolar = false;

    /**
    * we store this without type so that clients don't need
    * to pull in the kiss_fft headers. It's mutable so it can
    * be lazy created by FFT functions.
    * Note that the cfg has a "direction" baked into it. For
    * now we assume that all FFT with complex input will be inverse FFTs.
    */
    mutable void * kiss_cfg = 0;
};

using FFTDataReal = FFTData<float>;
using FFTDataCpx = FFTData<cpx>;

template<typename T> int FFTData<T>::_count = 0;

template <typename T>
inline FFTData<T>::FFTData(int numBins) :
    buffer(numBins)
{
    ++_count;
}

template <typename T>
inline FFTData<T>::~FFTData()
{
    // We need to manually delete the cfg, since only "we" know
    // what type it is.
    if (kiss_cfg) {
        free(kiss_cfg);
    }
    --_count;
}

template <typename T>
inline T FFTData<T>::get(int index) const
{
    assert(index < (int) buffer.size() && index >= 0);
    return buffer[index];
}

template <typename T>
inline void FFTData<T>::set(int index, T value)
{
    assert(index < (int) buffer.size() && index >= 0);
    buffer[index] = value;
}

/**
 * Template specializations for polar. only supported for complex
 */
inline bool FFTData<cpx>::isPolar() const {
    return _isPolar;

}

inline void FFTData<cpx>::toPolar() {
    assert(!_isPolar);
   
    for (int i=0; i< size(); ++i) {
        cpx x = get(i);
        float mag = std::abs(x);
        float phase = std::arg(x);
        set(i, cpx(mag, phase));
    }
     _isPolar = true;
}

 inline std::pair<float, float> FFTData<cpx>::getMagAndPhase(int bin) const {
    assert(_isPolar);
    cpx temp = get(bin);
    return std::make_pair(temp.real(), temp.imag());
}