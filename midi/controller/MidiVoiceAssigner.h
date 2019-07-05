#pragma once

class MidiVoice;

class MidiVoiceAssigner
{
public:
    MidiVoiceAssigner(MidiVoice* vx, int maxVoices);
    void setNumVoices(int);
    MidiVoice* getNext(float pitch);
private:
    MidiVoice* const voices;
    const int maxVoices;
};