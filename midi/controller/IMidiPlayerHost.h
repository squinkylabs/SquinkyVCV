#pragma once

/**
 * Implemented by a class that wants to host a Midi Player
 */
class IMidiPlayerHost
{
public:
    virtual void setGate(int voice, bool gate) = 0;
    virtual void setCV(int voice, float pitch) = 0;
    virtual void onLockFailed() = 0;
    virtual ~IMidiPlayerHost() = default;
};

#if 0
class MidiLoopParams
{
public:
    MidiLoopParams(bool b, float s, float e) : enabled(b), startTime(s), endTime(e)
    {

    }
    bool enabled=false;
    float startTime=0;
    float endTime = 0;
};
#endif