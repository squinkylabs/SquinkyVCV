#pragma once

#include "MidiLock.h"

#include <assert.h>
#include <memory>

class MidiSequencer;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

/**
 * Asserts that some code locked the MidiLock
 */
class MLockTest
{
public:
    MLockTest(MidiSequencerPtr s) : lp(s->song->lock)
    {
        assert(!lp->dataModelDirty());
        assert(!lp->locked());
    }
    ~MLockTest()
    {
        assert(lp->dataModelDirty());
        assert(!lp->locked());
    }
private:
    MidiLockPtr lp;
};