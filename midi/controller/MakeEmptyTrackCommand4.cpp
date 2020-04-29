#include "MakeEmptyTrackCommand4.h"
#include "MidiLock.h"
#include "MidiSequencer4.h"
#include "MidiSong4.h"

#include <assert.h>

MakeEmptyTrackCommand4::MakeEmptyTrackCommand4(int track, int section, bool addTrackFlag, const char* operationName) :
    track(track), section(section), addTrackFlag(addTrackFlag)
{
    name = operationName;
}

Command4Ptr MakeEmptyTrackCommand4::createAddTrack(MidiSequencer4Ptr, int track, int section, float duration)
{
    return std::make_shared<MakeEmptyTrackCommand4>(track, section, true, "add section");
}

Command4Ptr MakeEmptyTrackCommand4::createRemoveTrack(MidiSequencer4Ptr, int track, int section, float duration)
{
    return std::make_shared<MakeEmptyTrackCommand4>(track, section, false, "remove section");
}

void MakeEmptyTrackCommand4::addTrack(MidiSequencer4Ptr seq, Sequencer4Widget* widget)
{
    MidiLocker l(seq->song->lock);
    auto tk = MidiTrack::makeEmptyTrack(seq->song->lock);
    seq->song->addTrack(track, section, tk);
}

void MakeEmptyTrackCommand4::removeTrack(MidiSequencer4Ptr seq, Sequencer4Widget*)
{
    MidiLocker l(seq->song->lock);
    seq->song->addTrack(track, section, nullptr);
}


void MakeEmptyTrackCommand4::execute(MidiSequencer4Ptr seq, Sequencer4Widget* widget)
{
    if (addTrackFlag) {
        addTrack(seq, widget);
    }
    else {
        removeTrack(seq, widget);
    }
}

void MakeEmptyTrackCommand4::undo(MidiSequencer4Ptr seq, Sequencer4Widget* widget)
{
  
    if (addTrackFlag) {
        removeTrack(seq, widget);
    }
    else {
        addTrack(seq, widget);
    };
}
