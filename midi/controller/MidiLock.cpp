#pragma once

#include "MidiLock.h"

#include <assert.h>


MidiLock::MidiLock()
{
    theLock = false;
    editorLockLevel = 0;
}

MidiLockPtr MidiLock::make()
{
    return std::make_shared<MidiLock>();
}

void MidiLock::editorLock()
{
    if (editorLockLevel == 0) {

        // poll to take lock
        for (bool done = false; !done; ) {
            done = tryLock();
        }
    }
    ++editorLockLevel;
}
void MidiLock::editorUnlock()
{
    if (--editorLockLevel == 0) {
        theLock = false;
    }
}

bool MidiLock::playerTryLock()
{
    // try once to take lock
    return tryLock();
}

void MidiLock::playerUnlock()
{
    assert(locked());
    theLock = false;
}

bool MidiLock::tryLock()
{
    bool expected = false;
    bool desired = true;
    bool ret = theLock.compare_exchange_weak(expected, desired);
    return ret;
}

bool MidiLock::locked() const
{
    return theLock;
}

/***********************************************************************/


MidiLocker::MidiLocker(MidiLockPtr l) : lock(l)
{
    lock->editorLock();
}


MidiLocker::~MidiLocker()
{
    lock->editorUnlock();
}