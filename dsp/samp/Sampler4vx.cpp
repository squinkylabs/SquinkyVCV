
#include "Sampler4vx.h"

#include "CompiledInstrument.h"
#include "PitchUtils.h"
#include "SInstrument.h"
#include "WaveLoader.h"

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

#ifdef _SAMPFM
float_4 Sampler4vx::step(const float_4& gates, float sampleTime, const float_4& lfm, bool lfmEnabled) {
    sampleTime_ = sampleTime;
    if (patch && waves) {
        simd_assertMask(gates);

        float_4 envelopes = adsr.step(gates, sampleTime);
        float_4 samples = player.step(lfm, lfmEnabled);
        // apply envelope and boost level
        return envelopes * samples * _outputGain();
    } else {
        return 0;
    }
    return 0.f;
}

void Sampler4vx::setExpFM(const float_4& value) {
    fmCV = value;
}
#endif

#ifndef _SAMPFM

float_4 Sampler4vx::step(const float_4& gates, float sampleTime) {
    sampleTime_ = sampleTime;
    if (patch && waves) {
        simd_assertMask(gates);

        float_4 envelopes = adsr.step(gates, sampleTime);
        float_4 samples = player.step();
        // apply envelope and boost level
        return envelopes * samples * _outputGain();
    } else {
        return 0;
    }
    return 0.f;
}
#endif

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
        SQINFO("could not get play info pitch %d vel%d", midiPitch, midiVelocity);
        player.clearSamples(channel);
        return;
    }

    WaveLoader::WaveInfoPtr waveInfo = waves->getInfo(patchInfo.sampleIndex);
    assert(waveInfo->valid);
    assert(waveInfo->numChannels == 1);
    player.setSample(channel, waveInfo->data, int(waveInfo->totalFrameCount));

    player.setGain(channel, patchInfo.gain);

    // I don't think this test cares what we set the player too

#ifdef _SAMPFM
    // TODO: we will need to do this on step calls, too, to handle dynamic modulation
    const float transposeCV = patchInfo.transposeV * 12 + 12 * fmCV[channel];
 //const float transposeCV = patchInfo.transposeV * 12;       // temp - put back like it was?

    const float transposeAmt = PitchUtils::semitoneToFreqRatio(transposeCV);

#if 0
    SQINFO("");
    SQINFO("trans from patch = %f trans from fm = %f", patchInfo.transposeV * 12, fmCV[channel]);
    SQINFO("total transCV %f", transposeCV);
    SQINFO("final amt, after exp = %f", transposeAmt);
    SQINFO("will turn on needs transpose for now...");
#endif
    //  player.setTranspose(channel, patchInfo.needsTranspose, transposeAmt);
    player.setTranspose(channel, true, transposeAmt);
#else
    player.setTranspose(channel, patchInfo.needsTranspose, patchInfo.transposeAmt);
#endif

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
