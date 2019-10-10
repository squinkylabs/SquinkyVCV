#pragma once


#include "SqCommand.h"

#include <memory>

class NewSongDataDataCommand;

using NewSongDataDataCommandPtr = std::shared_ptr<NewSongDataDataCommand>;

class NewSongDataDataCommand : public SqCommand
{
public:
    virtual void execute(MidiSequencerPtr) override;
    virtual void undo(MidiSequencerPtr) override;
    static NewSongDataDataCommandPtr makeLoadMidiFileCommand();
private:
};