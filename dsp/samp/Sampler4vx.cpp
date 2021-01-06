
#include "Sampler4vx.h"

#include "CompiledInstrument.h"
#include "SInstrument.h"
#include "WaveLoader.h"

void Sampler4vx::setPatch(CompiledInstrumentPtr inst) {
    patch = inst;
}

void Sampler4vx::setLoader(WaveLoaderPtr loader) {
    waves = loader;
    adsr.setA(.1f, 1);
    adsr.setD(.1f, 1);
    adsr.setS(1);
    adsr.setR(.1f, 1);
}

void Sampler4vx::note_on(int channel, int midiPitch, int midiVelocity) {
    if (!patch || !waves) {
        printf("4vx not intit\n");
        return;
    }
    VoicePlayInfo patchInfo;
    VoicePlayParameter params;
    params.midiPitch = midiPitch;
    params.midiVelocity = midiVelocity;
    patch->play(patchInfo, params);
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

    // this is a little messed up - the adsr should really have independent
    // settings for each channel. OK for now, though.
    R[channel] = patchInfo.ampeg_release;
    adsr.setR(R[channel], 1);

    printf("note_on ch=%d, pitch-%d, vel=%d sample=%d\n", channel, midiPitch, midiVelocity, patchInfo.sampleIndex);  fflush(stdout);
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
        float_4 envelopes = adsr.step(gates, sampleTime);
        return envelopes * player.step();
    } else {
        return 0;
    }
    return 0.f;
}