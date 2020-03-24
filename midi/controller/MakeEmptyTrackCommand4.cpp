#include "MakeEmptyTrackCommand4.h"
#include <assert.h>

Command4Ptr MakeEmptyTrackCommand4::create()
{
    return std::make_shared<MakeEmptyTrackCommand4>();
}


void MakeEmptyTrackCommand4::execute(MidiSequencer4Ptr seq, Sequencer4Widget* widget)
{
    printf("MakeEmptyTrackCommand4::execute does nothing\n");
}
void MakeEmptyTrackCommand4::undo(MidiSequencer4Ptr seq, Sequencer4Widget*)
{
    printf("MakeEmptyTrackCommand4::undo does nothing\n");
}