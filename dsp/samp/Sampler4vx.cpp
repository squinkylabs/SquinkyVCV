
#include "Sampler4vx.h"

#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "WaveLoader.h"

#if 0
Sampler4vx::Sampler4vx() {
    divn.setup(32, [this] {
        this->step_n();
    });
}
#endif

void Sampler4vx::setPatch(CompiledInstrumentPtr inst) {
    patch = inst;
}

void Sampler4vx::setLoader(WaveLoaderPtr loader) {
    waves = loader;
    adsr.setASec(.001f);
    adsr.setDSec(.1f);
    adsr.setS(1);
    adsr.setRSec(.3f);
}

#if 0
void Sampler4vx::step_n() {
    // only check time remaining if we have a patch
    if (waves) {
        float_4 remainingSamples = player.audioSamplesRemaining();
        assertNE(sampleTime_, 0);
        float_4 timeRemaining = remainingSamples * sampleTime_;
        shutOffNow_ = timeRemaining < releaseTime_;
    }
}
#endif

float_4 Sampler4vx::step(const float_4& gates, float sampleTime) {
    sampleTime_ = sampleTime;
    if (patch && waves) {
      //  divn.step();

     //   float_4 gates = SimdBlocks::ifelse(shutOffNow_, SimdBlocks::maskFalse(), _gates);
        simd_assertMask(gates);

        float_4 envelopes = adsr.step(gates, sampleTime);
        float_4 samples = player.step();

        //
#if 0
        const bool b = acc.go(samples[0]);
        if (b) {
            auto x = acc.get();
            SQINFO("g=%f, env=%f d=%f,%f", gates[0], envelopes[0], x.first, x.second);
            SQINFO(" _gates=%f, shutOffNow=%f", _gates[0], shutOffNow_[0]);
        }
#endif
        // apply envelope and boost level
        return envelopes * samples * _outputGain();
    } else {
        return 0;
    }
    return 0.f;
}

void Sampler4vx::note_on(int channel, int midiPitch, int midiVelocity, float sampleRate) {
    if (!patch || !waves) {
        SQDEBUG("4vx not intit");
        return;
    }
    VoicePlayInfo patchInfo;
    VoicePlayParameter params;
    params.midiPitch = midiPitch;
    params.midiVelocity = midiVelocity;
    patch->play(patchInfo, params, waves.get(), sampleRate);
    if (!patchInfo.valid) {
        printf("could not get play info pitch %d vel%d\n", midiPitch, midiVelocity);
        fflush(stdout);
        return;
    }

  //  this->shutOffNow_[channel] = 0;
    WaveLoader::WaveInfoPtr waveInfo = waves->getInfo(patchInfo.sampleIndex);
    assert(waveInfo->valid);
    assert(waveInfo->numChannels == 1);
    player.setSample(channel, waveInfo->data, int(waveInfo->totalFrameCount));
    player.setTranspose(channel, patchInfo.needsTranspose, patchInfo.transposeAmt);
    player.setGain(channel, patchInfo.gain);

    std::string sample = waveInfo->fileName.getFilenamePart();
    // SQINFO("play vel=%d pitch=%d gain=%f samp=%s", midiVelocity, midiPitch, patchInfo.gain, sample.c_str());

    // this is a little messed up - the adsr should really have independent
    // settings for each channel. OK for now, though.
    R[channel] = patchInfo.ampeg_release;
    adsr.setRSec(R[channel]);
    releaseTime_ = patchInfo.ampeg_release;
}

void Sampler4vx::setNumVoices(int voices) {
}
