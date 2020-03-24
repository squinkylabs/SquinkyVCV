#pragma once

#include "SqCommand.h"

class MakeEmptyTrackCommand4 : public Sq4Command
{
public:
    static Command4Ptr createAddTrack(MidiSequencer4Ptr, int track, int section, float duration);
    static Command4Ptr createRemoveTrack(MidiSequencer4Ptr, int track, int section, float duration);
    void execute(MidiSequencer4Ptr seq, Sequencer4Widget* widget) override;
    void undo(MidiSequencer4Ptr seq, Sequencer4Widget*) override;

    MakeEmptyTrackCommand4(int track, int section, bool addTrackFlag, const char* operationName); 
private:
    const int track;
    const int section;
    const bool addTrackFlag;

    void addTrack(MidiSequencer4Ptr seq, Sequencer4Widget* widget);
    void removeTrack(MidiSequencer4Ptr seq, Sequencer4Widget* widget);
};
