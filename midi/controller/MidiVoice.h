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
     * Also, voice should have already been updated to the start time of the note.
     */
    void playNote(float pitch, float endTime);

    /**
     * Advance clock, sending any data that needs to be sent.
     * @param quarterNotes is the absolute metric time, in our standard quarter note units.
     */
    void updateToMetricTime(double quarterNotes);

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

    void setGate(bool);
    void setCV(float);
};