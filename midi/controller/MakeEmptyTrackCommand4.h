#pragma once

#include "SqCommand.h"

class MakeEmptyTrackCommand4 : public Sq4Command
{
public:
    static Command4Ptr create(MidiSequencer4Ptr, int track, int section, float duration);
    void execute(MidiSequencer4Ptr seq, Sequencer4Widget* widget) override;
    void undo(MidiSequencer4Ptr seq, Sequencer4Widget*) override;

     MakeEmptyTrackCommand4(int track, int section); // why public?
private:
    const int track;
    const int section;
};
