
#include "Sampler4vx.h"
#include "SInstrument.h"
#include "WaveLoader.h"


 void Sampler4vx::setPatch(SInstrumentPtr inst) {
    patch = inst;
  //  tryInit();
}

void Sampler4vx::setLoader(WaveLoaderPtr loader) {
    waves = loader;
   // tryInit();
}

//void Sampler4vx::stryInit() {
   // if (patch && waves) {
  //      stream
  //  }
//}

void Sampler4vx::note_on(int channel, int midiPitch, int midiVelocity)
{
    printf("note on\n"); fflush(stdout);
    if (!patch || !waves) {
        printf("4vx not intit\n");
        return;
    }
    SVoicePlayInfo patchInfo;
    patch->getInfo(patchInfo, midiPitch, midiVelocity);
    if (!patchInfo.valid) {
        printf("could not get play info\n");
        return;
    }
    WaveLoader::WaveInfoPtr waveInfo = waves->getInfo(patchInfo.sampleIndex);
    assert(waveInfo->valid);
    assert(waveInfo->numChannels == 1);
    player.setSample(waveInfo->data, int(waveInfo->totalFrameCount));
   //printf("finish me\n");
}

void Sampler4vx::note_off(int channel)
{

}

void Sampler4vx::setNumVoices(int voices)
{

}

float_4 Sampler4vx::step() {
    if (patch && waves) {
        return player.step();
    }
    else {
         printf("samp step not patch and waves\n"); fflush(stdout);
        // assert(false);      // always set the patch and waves first
        return 0;
    }
    return 0.f;
}