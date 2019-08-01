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
        const std::vector<MidiEventPtr>& inAdd,
        float trackLength = -1);

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
    static ReplaceDataCommandPtr makeChangeDurationCommand(std::shared_ptr<MidiSequencer> seq,  const std::vector<float>&);
    static ReplaceDataCommandPtr makePasteCommand(std::shared_ptr<MidiSequencer> seq);

    static ReplaceDataCommandPtr makeMoveEndCommand(std::shared_ptr<MidiSequencer> seq, float newLength);


private:

    int trackNumber;
    std::vector<MidiEventPtr> removeData;
    std::vector<MidiEventPtr> addData;

    /**
     * Clients who want track length changed should
     * pass in a non-negative value
     */
    float newTrackLength=-1;
    float originalTrackLength=-1;

    static void extendTrackToMinDuration(
        std::shared_ptr<MidiSequencer> seq,
        float neededLength,
        std::vector<MidiEventPtr>& toAdd,
        std::vector<MidiEventPtr>& toDelete);

    /**
     * queues up note additions and deletions to make them all fit in a shorter track
     */
    static void modifyNotesToFitNewLength(
        std::shared_ptr<MidiSequencer>seq,
        float newLengh, 
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

