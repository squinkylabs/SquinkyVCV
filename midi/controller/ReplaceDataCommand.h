#pragma once

#include <vector>

#include "MidiEvent.h"
#include "SqCommand.h"

class MidiEvent;
class MidiSong;

class ReplaceDataCommand : public SqCommand
{
public:
    virtual void execute() override;
    virtual void undo() override;

    // TODO: rvalue
    ReplaceDataCommand(
        std::shared_ptr<MidiSong> song,
        int trackNumber,
        const std::vector<MidiEvent>& inRemove,
        const std::vector<MidiEvent>& inAdd);

private:
    std::shared_ptr<MidiSong> song;
    int trackNumber;
    std::vector<MidiEvent> removeData;
    std::vector<MidiEvent> addData;

};