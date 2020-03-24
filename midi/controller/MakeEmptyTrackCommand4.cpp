#include "MakeEmptyTrackCommand4.h"
#include "MidiLock.h"
#include "MidiSequencer4.h"
#include "MidiSong4.h"

#include <assert.h>



MakeEmptyTrackCommand4::MakeEmptyTrackCommand4(int track, int section) :
    track(track), section(section)
{
    
}

/*
   MidiLocker l(song->lock);
        tk = MidiTrack::makeEmptyTrack(song->lock);
        song->addTrack(row, col, tk);
*/
Command4Ptr MakeEmptyTrackCommand4::create(MidiSequencer4Ptr, int track, int section, float duration)
{
    return std::make_shared<MakeEmptyTrackCommand4>(track, section);
}


void MakeEmptyTrackCommand4::execute(MidiSequencer4Ptr seq, Sequencer4Widget* widget)
{
    MidiLocker l(seq->song->lock);
    auto tk = MidiTrack::makeEmptyTrack(seq->song->lock);
    seq->song->addTrack(track, section, tk);
    printf("MakeEmptyTrackCommand4::execute does nothing\n");
}
void MakeEmptyTrackCommand4::undo(MidiSequencer4Ptr seq, Sequencer4Widget*)
{
    printf("MakeEmptyTrackCommand4::undo does nothing\n");
}