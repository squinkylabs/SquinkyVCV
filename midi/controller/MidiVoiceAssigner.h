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
    void reset();
private:
    MidiVoice* const voices;
    const int maxVoices;
    int numVoices = 0;
    int nextVoice = 0;

    Mode mode = Mode::ReUse;

    MidiVoice* getNextReUse(float pitch);
    int wrapAround(int vxNum);
    int advance(int vxNum);
};