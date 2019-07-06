#pragma once

class IMidiPlayerHost
{
public:
    virtual void setGate(int voice, bool gate) = 0;
    virtual void setCV(int voice, float pitch) = 0;
    virtual void onLockFailed() = 0;
};