#pragma once

#include <atomic>
class MidiLock
{
public:
    MidiLock();
    MidiLock(const MidiLock&) = delete;
    const MidiLock& operator = (const MidiLock&) = delete;
    void editorLock();
    void editorUnlock();

    bool playerTryLock();
    void playerUnlock();

    bool locked() const;
private:
    std::atomic<bool> theLock = false;
    bool editorDidLock = false;

    bool tryLock();
};