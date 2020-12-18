
#include "CompiledInstrument.h"
#include "Sampler4vx.h"
#include "SInstrument.h"
#include "WaveLoader.h"


 void Sampler4vx::setPatch(ci::CompiledInstrumentPtr inst) {
    patch = inst;
}

void Sampler4vx::setLoader(WaveLoaderPtr loader) {
    waves = loader;
}

void Sampler4vx::note_on(int channel, int midiPitch, int midiVelocity)
{
    if (!patch || !waves) {
        printf("4vx not intit\n");
        return;
    }
    ci::VoicePlayInfo patchInfo;
    patch->getInfo(patchInfo, midiPitch, midiVelocity);
    if (!patchInfo.valid) {
        printf("could not get play info\n");
        return;
    }
    WaveLoader::WaveInfoPtr waveInfo = waves->getInfo(patchInfo.sampleIndex);
    assert(waveInfo->valid);
    assert(waveInfo->numChannels == 1);
    player.setSample(waveInfo->data, int(waveInfo->totalFrameCount));
}

void Sampler4vx::note_off(int channel)
{
    player.mute();
}

void Sampler4vx::setNumVoices(int voices)
{
}

float_4 Sampler4vx::step() {
    if (patch && waves) {
        return player.step();
    }
    else {
        return 0;
    }
    return 0.f;
}