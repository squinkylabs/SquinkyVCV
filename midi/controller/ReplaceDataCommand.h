#pragma once

#include <vector>

#include "MidiEvent.h"
#include "SqCommand.h"

class MidiEditorContext;
class MidiEvent;
class MidiSong;
class MidiSequencer;
class MidiSelectionModel;
class ReplaceDataCommand;

using ReplaceDataCommandPtr = std::shared_ptr<ReplaceDataCommand>;

class ReplaceDataCommand : public SqCommand
{
public:
    virtual void execute() override;
    virtual void undo() override;

    // TODO: rvalue
    ReplaceDataCommand(
        std::shared_ptr<MidiSong> song,
        std::shared_ptr<MidiSelectionModel>,
        int trackNumber,
        const std::vector<MidiEventPtr>& inRemove,
        const std::vector<MidiEventPtr>& inAdd);

    /**
     * static factories for replace commands
     */
    static ReplaceDataCommandPtr makeDeleteCommand(std::shared_ptr<MidiSequencer> seq);
    static ReplaceDataCommandPtr makeInsertNoteCommand(std::shared_ptr<MidiSequencer> seq);
    static ReplaceDataCommandPtr makeChangePitchCommand(std::shared_ptr<MidiSequencer> seq, int semitones);


private:
    std::shared_ptr<MidiSong> song;
    int trackNumber;
    std::shared_ptr<MidiSelectionModel> selection;

    std::vector<MidiEventPtr> removeData;
    std::vector<MidiEventPtr> addData;
};

