#pragma once

class MidiVoice;

class MidiVoiceAssigner
{
public:
    enum class Mode
    {
        ReUse,          // default
        Rotate
    };
    MidiVoiceAssigner(MidiVoice* vx, int maxVoices);
    void setNumVoices(int);
    MidiVoice* getNext(float pitch);
private:
    MidiVoice* const voices;
    const int maxVoices;

    Mode mode = Mode::ReUse;

    MidiVoice* getNextReUse(float pitch);
};