
#include "FFTUtils.h"
#include "AudioMath.h"

std::vector< FFTDataCpxPtr> FFTUtils::generateFFTs(int numSamples, int frameSize, std::function<double()> generator)
{
    auto data = generateData(numSamples, frameSize, generator);
    std::vector<FFTDataCpxPtr> ret;
    for (auto buffer : data) {
        FFTDataCpxPtr  fft = std::make_shared<FFTDataCpx>(frameSize);
        FFT::forward(fft.get(), *buffer);
        ret.push_back(fft);
    }
    return ret;
}

std::vector< FFTDataRealPtr> FFTUtils::generateData(int numSamples, int frameSize, std::function<double()> generator)
{
    std::vector< FFTDataRealPtr> ret;
    FFTDataRealPtr buffer;
    int index = 0;
    while (numSamples--) {
        if (!buffer) {
            buffer = std::make_shared<FFTDataReal>(frameSize);
            ret.push_back(buffer);
            index = 0;
        }
        float x = (float) generator();
        buffer->set(index, x);
        ++index;
        if (index >= frameSize) {
            buffer.reset();
        }
    }
    return ret;
}


// this version does the angle math in doubles
void FFTUtils::getStats2(Stats& stats, const FFTDataCpx& a, const FFTDataCpx& b, const FFTDataCpx& c)
{
    printf("fftUtils::getStats\n");
    assert(a.size() == b.size());
    assert(a.size() == c.size());
    assert(!a.isPolar() && !b.isPolar() && !c.isPolar());

    double biggestJump = 0;
    double magSum = 0;
    double magSumNot10 = 0;
    double weightedJumpSum = 0;
    
    // skip the DC bin. a) the phase is totally meainingless. b) who wants DC in here?
    for (int bin = 1; bin < a.size(); ++bin) {
        const bool print = false;
        
       // auto mpa = a.getMagAndPhase(bin);
       // auto mpb = b.getMagAndPhase(bin);
      //  auto mpc = c.getMagAndPhase(bin);
        std::complex<double> an = a.get(bin);
        const double maga = std::abs(an);
        const double phasea = std::arg(an);

        std::complex<double> bn = b.get(bin);
        const double magb = std::abs(bn);
        const double phaseb = std::arg(bn);

        std::complex<double> cn = c.get(bin);
        const double magc = std::abs(cn);
        const double phasec = std::arg(cn);

        const double phaseDiff0 = PhaseAngleUtil::distance(phaseb, phasea, print);
        const double phaseDiff1 = PhaseAngleUtil::distance(phasec, phaseb, print);

        const double mag = maga;
        const double jump = std::abs(PhaseAngleUtil::distance(phaseDiff1,  phaseDiff0));

       // if (print) {
        if (mag > .01) {
            printf("bin %d mag %f jump=%f, ph = %f, %f, %f\n", bin, mag, jump, phasea, phaseb, phasec);
           // printf("   first dif = %.2f, second dif = %.2f\n", phaseDiff0, phaseDiff1);
            //printf(" ph (norm to +-1) = %.2f, %.2f, %.2f\n", mpa.second / AudioMath::Pi, mpb.second / AudioMath::Pi, mpc.second / AudioMath::Pi);
        }
        assert(mag >= 0);
        magSum += mag;
        weightedJumpSum += (jump * mag);
        if (bin != 10 && bin != 9 && bin != 11) {
            magSumNot10 += mag;
        }
    }
    printf("total shift %f mag %f mag note 10 = %f\n", weightedJumpSum, magSum, magSumNot10);
    double totalJump = (magSum > 0) ? weightedJumpSum / magSum : 0;
    stats.averagePhaseJump = totalJump;
}

void FFTUtils::getStats(Stats& stats, const FFTDataCpx& a, const FFTDataCpx& b, const FFTDataCpx& c)
{
    printf("fftUtils::getStats\n");
    assert(a.size() == b.size());
    assert(a.size() == c.size());
    assert(a.isPolar() && b.isPolar() && c.isPolar());

    double biggestJump = 0;
    double magSum = 0;
    double weightedJumpSum = 0;
    
    // skip the DC bin. a) the phase is totally meainingless. b) who wants DC in here?
    for (int bin = 1; bin < a.size(); ++bin) {

       // const bool print = (bin == 9);
       const bool print = false;
        auto mpa = a.getMagAndPhase(bin);
        auto mpb = b.getMagAndPhase(bin);
        auto mpc = c.getMagAndPhase(bin);

        if (print) {
            printf("bin 9, raw phases are = %.2f, %.2f, %.2f\n", mpa.second, mpb.second, mpc.second);
            printf("  raw diffs %2.f, %.2f\n", (mpb.second - mpa.second), (mpc.second - mpb.second));
            printf("  raw diffs %f, %f\n", (mpb.second - mpa.second), (mpc.second - mpb.second));

            const double x = mpa.second - 2 * mpb.second + mpc.second;
            double y = x;
            if (x < 0) {
                y += AudioMath::_2Pi;
            }
            printf("  a-2b+c = %f (%f)\n", x, y);
        }

       

        const double phaseDiff0 = PhaseAngleUtil::distance(mpb.second, mpa.second, print);
        const double phaseDiff1 = PhaseAngleUtil::distance(mpc.second, mpb.second, print);

        const double mag = mpa.first;
        const double jump = std::abs(PhaseAngleUtil::distance(phaseDiff1,  phaseDiff0));

       // if (print) {
        if (mag > .01) {
            printf("bin %d mag %f jump=%f, ph = %f, %f, %f\n", bin, mag, jump, mpa.second, mpb.second, mpc.second);
           // printf("   first dif = %.2f, second dif = %.2f\n", phaseDiff0, phaseDiff1);
            //printf(" ph (norm to +-1) = %.2f, %.2f, %.2f\n", mpa.second / AudioMath::Pi, mpb.second / AudioMath::Pi, mpc.second / AudioMath::Pi);
        }
        assert(mag >= 0);
        magSum += mag;
        weightedJumpSum += (jump * mag);

    }
    printf("total shift %f mag %f\n", weightedJumpSum, magSum);
    double totalJump = (magSum > 0) ? weightedJumpSum / magSum : 0;
    stats.averagePhaseJump = totalJump;
    
}
