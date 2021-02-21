
#include "Sampler4vx.h"

#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "WaveLoader.h"

void Sampler4vx::setPatch(CompiledInstrumentPtr inst) {
    patch = inst;
}

void Sampler4vx::setLoader(WaveLoaderPtr loader) {
    waves = loader;
#ifdef _USEADSR
    adsr.setA(.1f, 1);
    adsr.setD(.1f, 1);
    adsr.setS(1);
    adsr.setR(.1f, 1);
#endif
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
    R[channel] = patchInfo.ampeg_release;
    //adsr.setR(R[channel], 1);
    adsr.setR_L(R[channel]);
#endif

    // printf("note_on ch=%d, pitch-%d, vel=%d sample=%d\n", channel, midiPitch, midiVelocity, patchInfo.sampleIndex);  fflush(stdout);
}

void Sampler4vx::note_off(int channel) {
    player.mute(channel);
    // printf("note off %d\n", channel);  fflush(stdout);
}

void Sampler4vx::setNumVoices(int voices) {
}

float_4 Sampler4vx::step(const float_4& gates, float sampleTime) {
    if (patch && waves) {
        // float_4 step(const float_4& gates, float sampleTime);
#ifdef _USEADSR
        simd_assertMask(gates);

        float_4 envelopes = adsr.step(gates, sampleTime);
        float_4 samples = player.step();
        //printf("eg0 = %f samp0 = %f\n", envelopes[0], samples[0]);

        // apply envelope and boost level
        return envelopes * samples * float_4(5);
#else
        float_4 samples = player.step();
        return samples;
#endif
    } else {
        return 0;
    }
    return 0.f;
}