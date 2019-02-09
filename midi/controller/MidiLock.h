#pragma once

#include <atomic>
#include <memory>

class MidiLock;
using MidiLockPtr = std::shared_ptr<MidiLock>;

class MidiLock
{
public:
    MidiLock();
    MidiLock(const MidiLock&) = delete;
    const MidiLock& operator = (const MidiLock&) = delete;
    static MidiLockPtr make();
    void editorLock();
    void editorUnlock();

    bool playerTryLock();
    void playerUnlock();

    bool locked() const;
private:
    std::atomic<bool> theLock;
    std::atomic<int> editorLockLevel;
    bool editorDidLock = false;

    bool tryLock();
};

class MidiLocker
{
public:
    MidiLocker(MidiLockPtr);
    ~MidiLocker();
private:
    MidiLockPtr lock;
};

