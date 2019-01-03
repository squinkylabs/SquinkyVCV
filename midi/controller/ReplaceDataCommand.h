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
        const std::vector<MidiEventPtr>& inRemove,
        const std::vector<MidiEventPtr>& inAdd);

private:
    std::shared_ptr<MidiSong> song;
    int trackNumber;
    std::vector<MidiEventPtr> removeData;
    std::vector<MidiEventPtr> addData;

};