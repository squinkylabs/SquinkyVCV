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

    /** 
     * Returns true is an editor lock has occurred since last query.
     * Will clear flag after calling
     */
    bool dataModelDirty();

private:
    std::atomic<bool> theLock;
    std::atomic<int> editorLockLevel;
    std::atomic<bool> editorDidLock;

    bool tryLock();
};

/**
 * MidiLocker is a scoped lock.
 * It calls editorLock and editorUnlock, so may
 * only be used by the UI thread.
 */
class MidiLocker
{
public:
    MidiLocker(MidiLockPtr);
    ~MidiLocker();
private:
    MidiLockPtr lock;
};

