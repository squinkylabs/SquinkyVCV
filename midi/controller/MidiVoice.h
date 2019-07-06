#pragma once

class IMidiPlayerHost;

class MidiVoice
{
public:
    enum class State {Idle, Playing, ReTriggering };
    void setHost(IMidiPlayerHost*);
    void setIndex(int);

    /**
     * Will always play the note, not matter what state it is in.
     * Clever voice assignment should have been done long before calling this.
     */
    void playNote(float pitch, float endTime);

    // TODO: generalize to state()
   // bool isPlaying() const;
    State state() const;
    float pitch() const;
private:
    double noteOffTime = -1;        // the absolute metric time when the 
                                    // currently playing note should stop
    float curPitch= -100;           // the pitch of the last note played in this voice

    IMidiPlayerHost* host = nullptr;

    State curState = State::Idle;
    int index = 0;
};