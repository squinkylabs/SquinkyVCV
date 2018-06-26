#include <assert.h>

#include "Analyzer.h"
#include "AudioMath.h"
#include "FFT.h"
#include "FFTData.h"
#include "SinOscillator.h"
#include "AudioMath.h"


int Analyzer::getMax(const FFTDataCpx& data)
{
    int maxBin = -1;
    float maxMag = 0;
    for (int i = 0; i < data.size(); ++i) {
        const float mag = std::abs(data.get(i));
        if (mag > maxMag) {
            maxMag = mag;
            maxBin = i;
        }
    }
    return maxBin;
}

float Analyzer::getSlope(const FFTDataCpx& response, float fTest, float sampleRate)
{
    return 0;
}

std::tuple<int, int, int> Analyzer::getMaxAndShoulders(const FFTDataCpx& data, float atten)
{
    assert(atten < 0);
    int maxBin = getMax(data);
    int iMax = data.size() / 2;

    assert(maxBin >= 0);
    const double dbShoulder = atten +  AudioMath::db(std::abs(data.get(maxBin)));

    int i;
    int iShoulderLow = -1;
    int iShoulderHigh = -1;
    bool done;
    for (done = false, i = maxBin; !done; ) {
        const double db = AudioMath::db(std::abs(data.get(i)));
        if (i >= iMax) {
            done = true;
        } else if (db <= dbShoulder) {
            iShoulderHigh = i;
            done = true;
        } else {
            i++;
        }
    }
    for (done = false, i = maxBin; !done; ) {
        const double db = AudioMath::db(std::abs(data.get(i)));
        if (db <= dbShoulder) {
            iShoulderLow = i;
            done = true;
        } else if (i <= 0) {
            done = true;
        } else {
            i--;
        }
    }
    printf("out of loop, imax=%d, shoulders=%d,%d\n", maxBin, iShoulderLow, iShoulderHigh);

    return std::make_tuple(iShoulderLow, maxBin, iShoulderHigh);
}

std::vector<Analyzer::FPoint> Analyzer::getFeatures(const FFTDataCpx& data, float sensitivityDb, float sampleRate)
{
    std::vector<FPoint> ret;
    float lastDb = 10000000000;
    // only look at the below nyquist stuff
    for (int i = 0; i < data.size()/2; ++i) {
        const float db = (float) AudioMath::db( std::abs(data.get(i)));
        if (std::abs(db - lastDb) >= sensitivityDb) {
            float freq = FFT::bin2Freq(i, sampleRate, data.size());
            FPoint p(freq, db);
           // printf("feature at bin %d, db=%f raw val=%f\n", i, db, std::abs(data.get(i)));
            ret.push_back(p);
            lastDb = db;
        }
    }
    return ret;
}

void Analyzer::getAndPrintFeatures(const FFTDataCpx& data, float sensitivityDb, float sampleRate)
{
    auto features = getFeatures(data, sensitivityDb, sampleRate);
    printf("there are %d features\n", (int)features.size());
    for (int i = 0; i < features.size(); ++i) {
        printf("feature: freq=%f, db=%f\n", features[i].freq, features[i].gain);
    }
}

void Analyzer::getFreqResponse(FFTDataCpx& out, std::function<float(float)> func)
{
    /**
     * testSignal is the time domain sweep
     * testOutput if the time domain output of "func"
     * testSpecrum is the FFT of testSignal
     * spectrum is the FFT of testOutput

     */
    // First set up a test signal 
    const int numSamples = out.size();
  //  std::vector<float> testSignal(numSamples);
    FFTDataReal testSignal(numSamples);
    generateSweep(44100, testSignal.data(), numSamples, 20, 20000);

    // Run the test signal though func, capture output in fft real
    FFTDataReal testOutput(numSamples);
    for (int i = 0; i < out.size(); ++i) {
        const float y = func(testSignal.get(i));
        testOutput.set(i, y);
    }

    // then take the inverse fft
    FFTDataCpx spectrum(numSamples);
    FFT::forward(&spectrum, testOutput);

   
    // then divide by test
    FFTDataCpx testSpectrum(numSamples);
    FFT::forward(&testSpectrum, testSignal);

    for (int i = 0; i < numSamples; ++i) {

        const cpx x =  (std::abs(testSpectrum.get(i)) == 0) ? 0 :
            spectrum.get(i) / testSpectrum.get(i);
        out.set(i, x);
    }

#if 0
    for (int i = 0; i < numSamples; ++i) {
        printf("%d, sig=%f out=%f mag(sig)=%f mag(out)=%f rsp=%f\n",
            i, testSignal.get(i), testOutput.get(i),
            std::abs(testSpectrum.get(i)),
            std::abs(spectrum.get(i)),
            std::abs(out.get(i))
        );
    }
#endif
}


void Analyzer::generateSweep(float sampleRate, float* out, int numSamples, float minFreq, float maxFreq)
{
    assert(maxFreq > minFreq);
    const double minLog = std::log2(minFreq);
    const double maxLog = std::log2(maxFreq);
    const double delta = (maxLog - minLog) / numSamples;

    SinOscillatorParams<double> params;
    SinOscillatorState<double> state;

    double fLog = minLog;
    for (int i = 0; i < numSamples; ++i, fLog+=delta) {
        const double freq = std::pow(2, fLog);
        assert(freq < (sampleRate / 2));

        SinOscillator<double, false>::setFrequency(params, freq / sampleRate);
        double val = SinOscillator<double, false>::run(state, params);

       // ::printf("out[%d] = %f f=%f\n", i, val, freq);
        out[i] = (float) val;
    }
}