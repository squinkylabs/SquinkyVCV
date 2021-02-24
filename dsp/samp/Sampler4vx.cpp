
#include "Sampler4vx.h"

#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "WaveLoader.h"

Sampler4vx::Sampler4vx() {
    divn.setup(32, [this]{
        this->step_n();
    });
}
void Sampler4vx::setPatch(CompiledInstrumentPtr inst) {
    patch = inst;
}

void Sampler4vx::setLoader(WaveLoaderPtr loader) {
    waves = loader;
#ifdef _USEADSR
    adsr.setASec(.1f);
    adsr.setDSec(.1f);
    adsr.setS(1);
    adsr.setRSec(.3f);
#endif
}


void Sampler4vx::step_n() {
    float_4 remainingSamples = player.audioSamplesRemaining();
    assertNE(sampleTime_, 0);
    float_4 timeRemaining = remainingSamples * sampleTime_;

    shutOffNow_ = timeRemaining < releaseTime_;
}

float_4 Sampler4vx::step(const float_4& _gates, float sampleTime) {
    sampleTime_ = sampleTime;
    if (patch && waves) {
        divn.step();
#ifdef _USEADSR
        float_4 gates = SimdBlocks::ifelse(shutOffNow_, SimdBlocks::maskFalse(), _gates);
        simd_assertMask(gates);

        float_4 envelopes = adsr.step(gates, sampleTime);
        float_4 samples = player.step();

       // SQINFO("g=%f, env=%f", gates[0], envelopes[0]);

        // apply envelope and boost level
        return envelopes * samples * _outputGain();;
#else
        float_4 samples = player.step();
        return samples;
#endif
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

    WaveLoader::WaveInfoPtr waveInfo = waves->getInfo(patchInfo.sampleIndex);
    assert(waveInfo->valid);
    assert(waveInfo->numChannels == 1);
    player.setSample(channel, waveInfo->data, int(waveInfo->totalFrameCount));
    player.setTranspose(channel, patchInfo.needsTranspose, patchInfo.transposeAmt);
    player.setGain(channel, patchInfo.gain);

    // this is a little messed up - the adsr should really have independent
    // settings for each channel. OK for now, though.
#ifdef _USEADSR
   // SQINFO("setting release to %f, vel = %d", patchInfo.ampeg_release, midiVelocity);

    // This is all super janky how we setl all 4 adsr to same TC.
    // But good enough for now
    R[channel] = patchInfo.ampeg_release;
    adsr.setRSec(R[channel]);
    releaseTime_ = patchInfo.ampeg_release;
#endif
}

#ifndef _USEADSR
void Sampler4vx::note_off(int channel) {
    player.mute(channel);
}
#endif

void Sampler4vx::setNumVoices(int voices) {
}

