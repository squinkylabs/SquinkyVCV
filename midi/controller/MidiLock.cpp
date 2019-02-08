#pragma once

#include "MidiLock.h"

#include <assert.h>


MidiLock::MidiLock()
{

}
void MidiLock::editorLock()
{
    // poll to take lock
    for (bool done = false; !done; ) {
        done = tryLock();
    }
}
void MidiLock::editorUnlock()
{
    assert(theLock);
    theLock = false;
    // free lock, dirty data
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