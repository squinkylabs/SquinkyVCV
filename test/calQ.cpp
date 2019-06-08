#include "Analyzer.h"
#include "AudioMath.h"
#include "LadderFilter.h"
#include <stdio.h>
#include <map>
#include <memory>

using T = double;
using Filt = LadderFilter<T>;
using FiltPtr = std::shared_ptr<Filt>;

// was trying 64k
const int numBins =  4 * 16 * 4096;

// 
const double toleranceDb = 1;
const double toleranceDbInterp = 10;
const double sampleRate = 44100;

//ok at 100
const double desiredGain = 40;
const double desiredGainDb = AudioMath::db(desiredGain);

FiltPtr getFilter(T normFc, T feedback)
{
    FiltPtr filter = std::make_shared<Filt>();
  
    filter->setNormalizedFc(normFc);
    filter->setVoicing(Filt::Voicing::Clean);
    filter->disableQComp();
    filter->setFeedback(feedback);
    
    const double d0 = filter->getOutput();
    assert(d0 == 0);
    filter->run(0);

    const double d1 = filter->getOutput();
    assert(d1 == 0);
 
   
    return filter;
}

double getPeakAmp(FiltPtr filter)
{

    std::function<float(float)> doFilter = [filter](float x) {
        filter->run(x);
        float value = (float) filter->getOutput();;
        return value;
    };

    FFTDataCpx fftData(numBins);
    Analyzer::getFreqResponse(fftData, doFilter);
    const int maxBin = Analyzer::getMax(fftData);

    assert(maxBin >= 2);
    assert(maxBin < (numBins - 1));


    // let's count the neighboring bins, in case we are split between two.
    double amp = fftData.getAbs(maxBin);

    if ((amp < fftData.getAbs(maxBin - 1)) || (amp < fftData.getAbs(maxBin + 1)))
    {
        std::string s = "biggest bin isn't biggest";
            throw(s);
    }

    // TODO: determine if this helps (it seems to make it worse
   // amp += fftData.getAbs(maxBin-1);
   // amp += fftData.getAbs(maxBin + 1);
    return amp;
}

double getPeakAmp(double normFc, double feedback)
{
   // printf("\ngetPeakAmp f=%f feedback=%f\n", normFc, feedback);
    double ret = 0;

    
    auto filter = getFilter(T(normFc), T(feedback));
    ret = getPeakAmp(filter);

  //  printf("getPeakAmp(%f, %f) = %f\n", normFc, feedback, ret);
    return ret;
}

double findFeedback(const double normFc)
{
    printf("\n------- findFeedback (%f)----\n", normFc * sampleRate);
    double feedbackMin = 1;
    double feedbackMax = 4;
  
    double feedback = 0;
    int times = 0;
    for (bool done = false; !done; ) {
        if (++times > 100) {
            //done = true;
            printf("can't find Q at %f. f=%f / %.40e\n",  normFc * sampleRate, feedback, normFc);
            throw std::string("can't find Q");
        }
        feedback = .5 * (feedbackMax + feedbackMin);
        const double amp = getPeakAmp(normFc, feedback);
        const double db = AudioMath::db(amp);
     //  printf("in ff loop targ = %.2f db=%f f=%f min=%f, max=%f\n", desiredGainDb, db, feedback, feedbackMin, feedbackMax);
        double err = std::abs(db - desiredGainDb);
        if (err < toleranceDb) {
            printf("found good Q after %d iterations feedback=%f f=%.2f measuredb=%.2f\n", times, feedback, normFc * sampleRate, db);
            double f2 = getPeakAmp(normFc, feedback);
            printf("measure second time, db = %f\n", AudioMath::db(f2));
            return feedback;
        }
        if (db > desiredGainDb) {
          //  printf("  errdb=%f too mucn gain, so bring max down\n", err);
            // if we are too high, don't want to go higher
            feedbackMax = std::min(feedback, feedbackMax);
        } else {
           // printf("  errdb=%f too little gain, so bring min up\n", err);
            feedbackMin = std::max(feedback, feedbackMin);
        }
    }
    assert(false);       // can't get here?
    return feedback;

}

double interpFreq(double low, double high)
{
    double logAvg = .5 * (log2(low) + log2(high));
    return exp2(logAvg);
}

std::map<double, double> data;
static void registerData(double freq, double fb)
{
  //  printf("register(%f) = %f\n", freq, fb);
    if (data.find(freq) != data.end()) {
       // printf("%f already reg\n", freq);
    }
    data[freq] = fb;
}

static bool isRegistered(double freq)
{
    return data.find(freq) != data.end();
}

static void dumpData()
{
    bool isFirst = true;
    std::map<double, double>::iterator it;
    for (it = data.begin(); it != data.end(); ++it) {

        const double f = it->first;
        const double fb = it->second;
        double db = AudioMath::db(getPeakAmp(f, fb));
        printf("f[%.2f / %f] fb=%.4f db = %.2f\n", f * sampleRate, f, fb, db);
#if 1
        std::map<double, double>::iterator it_next = it;
        ++it_next;
        if (it_next != data.end()) {
            double fTest = .5 * (f + it_next->first);
            double fbTest = .5 * (fb + it_next->second);
            db = AudioMath::db(getPeakAmp(fTest, fbTest));
            printf("  interp g = %.2f              ERR = %.2f\n", db, fabs(db - desiredGainDb));
        }
#endif
    }
    printf("there are %d entries log2=%f\n", (int) data.size(), std::log2( double(data.size())));
    printf("desired gainDb = %f\n", desiredGainDb);
}
static void doRange(double fLow, double fHigh, double spacing)
{
    for (double f = fLow; f <= fHigh; f *= spacing) {
#if 0
        double fb;
        try {
            fb = findFeedback(f);           // let's try just one
        }
        catch (std::string&) {
            printf("find feedback threw error, will skip %f\n", f);
            continue;
        }
        printf("find feedback no error %f\n", f);
#endif
        const double fb = findFeedback(f);
        registerData(f, fb);
    }
}

void cal2(double fLow, double feedLow, double fHigh, double feedHigh)
{
    assert(fLow <= sampleRate / 2);
    assert(fHigh <= sampleRate / 2);
    assert(fLow < feedHigh);
    assert(feedLow > feedHigh);

    registerData(fLow, feedLow);
    registerData(fHigh, feedHigh);

    double fMiddle = .5 * (fLow + fHigh);
    double feedMiddle = .5 * (feedLow + feedHigh);
    double dbMiddle = AudioMath::db(getPeakAmp(fMiddle, feedMiddle));
    double err = (dbMiddle - desiredGainDb);

    printf("in cal2(%.2f, %.2f, testing f=%.2f err = %f\n", fLow * sampleRate, fHigh * sampleRate, fMiddle * sampleRate, err);
    if (err <= toleranceDbInterp) {
        printf("success at %.2f\n", fMiddle * sampleRate);
        return;
    }

    // now we need accurate middle feedback
    feedMiddle = findFeedback(fMiddle);

    cal2(fLow, feedLow, fMiddle, feedMiddle);
    cal2(fMiddle, feedMiddle, fHigh, feedHigh);

}

void calQ()
{


   try {
       // doRange(50 / sampleRate, 1000/ sampleRate, 1.3);
      //  doRange(1000 / sampleRate, 5000 / sampleRate, 1.2);
       // doRange(5000 / sampleRate, 9800 / sampleRate, 1.05);
     // doRange(9800 / sampleRate, 22000 / sampleRate, 1.02);
     //  findFeedback(50 / sampleRate);
#if 1
       double fLow = 4000 / sampleRate;
       double fHigh = 20000 / sampleRate;
       double feedLow = findFeedback(fLow);
       double feedHigh = findFeedback(fHigh);
       cal2(fLow, feedLow, fHigh, feedHigh);
#endif
 
   }
   catch (std::string& err) {
       printf("\n\n----- ERROR %s -------\n", err.c_str());
   }
   dumpData();
}