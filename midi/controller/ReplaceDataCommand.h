#pragma once

#include <vector>

#include "MidiEvent.h"
#include "SqCommand.h"

class MidiEditorContext;
class MidiEvent;
class MidiNoteEvent;
class MidiSong;
class MidiSequencer;
class MidiSelectionModel;
class ReplaceDataCommand;

using ReplaceDataCommandPtr = std::shared_ptr<ReplaceDataCommand>;

class ReplaceDataCommand : public SqCommand
{
public:
    virtual void execute(MidiSequencerPtr) override;
    virtual void undo(MidiSequencerPtr) override;

    // TODO: get rid of obsolete arguments.
    ReplaceDataCommand(
        std::shared_ptr<MidiSong> song,
        std::shared_ptr<MidiSelectionModel>,
        std::shared_ptr<MidiEditorContext>,
        int trackNumber,
        const std::vector<MidiEventPtr>& inRemove,
        const std::vector<MidiEventPtr>& inAdd);

    ReplaceDataCommand(
        std::shared_ptr<MidiSong> song,
        int trackNumber,
        const std::vector<MidiEventPtr>& inRemove,
        const std::vector<MidiEventPtr>& inAdd);


    /**
     * static factories for replace commands
     */
    static ReplaceDataCommandPtr makeDeleteCommand(std::shared_ptr<MidiSequencer> seq, const char* name);
    static ReplaceDataCommandPtr makeInsertNoteCommand(std::shared_ptr<MidiSequencer> seq, std::shared_ptr<const MidiNoteEvent>);
    static ReplaceDataCommandPtr makeChangePitchCommand(std::shared_ptr<MidiSequencer> seq, int semitones);
    static ReplaceDataCommandPtr makeChangeStartTimeCommand(std::shared_ptr<MidiSequencer> seq, float delta);
    static ReplaceDataCommandPtr makeChangeStartTimeCommand(std::shared_ptr<MidiSequencer> seq, const std::vector<float>&);
    
    static ReplaceDataCommandPtr makeChangeDurationCommand(std::shared_ptr<MidiSequencer> seq, float delta);
    static ReplaceDataCommandPtr makePasteCommand(std::shared_ptr<MidiSequencer> seq);


private:
    // old way. can't save data pointers with vcv 1
    //std::shared_ptr<MidiSong> song;
    //std::shared_ptr<MidiSelectionModel> selection;

    int trackNumber;
    std::vector<MidiEventPtr> removeData;
    std::vector<MidiEventPtr> addData;

    static void extendTrackToMinDuration(
        std::shared_ptr<MidiSequencer> seq,
        float neededLength,
        std::vector<MidiEventPtr>& toAdd,
        std::vector<MidiEventPtr>& toDelete);

    // base for change pitch, start time, duration
    enum class Ops {Pitch, Start, Duration };
    using Xform = std::function<void(MidiEventPtr event, int index)>;
    static ReplaceDataCommandPtr makeChangeNoteCommand(
        Ops,
        std::shared_ptr<MidiSequencer> seq,
        Xform xform,
        bool canChangeLength);
};

