#pragma once

#include <memory>

/**
 * Implemented by a class that wants to host a Midi Player.
 * Single track players can ignore the track parameter.
 */
class IMidiPlayerHost4
{
public:
    virtual void setGate(int track, int voice, bool gate) = 0;
    virtual void setCV(int track, int voice, float pitch) = 0;
    virtual void onLockFailed() = 0;

    /**
     * when player calls resetCLock, host must actually 
     * go and reset the clock.
     */
    virtual void resetClock() = 0;
    virtual ~IMidiPlayerHost4() = default;
};

using IMidiPlayerHost4Ptr = std::shared_ptr<IMidiPlayerHost4>;


/**
 * Receiver for UI requests to audition a note.
 * Not really a host, but...
 */
class IMidiPlayerAuditionHost
{
public:
    virtual void auditionNote(float pitch) = 0;
    virtual ~IMidiPlayerAuditionHost() = default;
};

using IMidiPlayerAuditionHostPtr = std::shared_ptr<IMidiPlayerAuditionHost>;
