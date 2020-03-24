#include "MakeEmptyTrackCommand4.h"
#include <assert.h>

Command4Ptr MakeEmptyTrackCommand4::create()
{
    return std::make_shared<MakeEmptyTrackCommand4>();
}


void MakeEmptyTrackCommand4::execute(MidiSequencer4Ptr seq, Sequencer4Widget* widget)
{
    assert(false);
}
void MakeEmptyTrackCommand4::undo(MidiSequencer4Ptr seq, Sequencer4Widget*)
{
    assert(false);
}