#pragma once

#include "SqCommand.h"

class MakeEmptyTrackCommand4 : public Sq4Command
{
public:
    static Command4Ptr create();
    void execute(MidiSequencer4Ptr seq, Sequencer4Widget* widget) override;
    void undo(MidiSequencer4Ptr seq, Sequencer4Widget*) override;
};
