
#include "Sampler4vx.h"
#include "SInstrument.h"


void Sampler4vx::note_on(int channel, int midiPitch, int midiVelocity)
{
    if (!patch || !waves) {
        printf("4vx not intit\n");
        return;
    }
    SVoicePlayInfo info;
    patch->getInfo(info, midiPitch, midiVelocity);
    if (!info.valid) {
        printf("could not get play info\n");
        return;
    }
    printf("finish me\n");
}

void Sampler4vx::note_off(int channel)
{

}

void Sampler4vx::setNumVoices(int voices)
{

}

float_4 Sampler4vx::step() {
    return 0.f;
}