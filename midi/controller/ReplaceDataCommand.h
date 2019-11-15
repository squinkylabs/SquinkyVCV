#pragma once

#include <functional>
#include <vector>

#include "SqCommand.h"

class MidiEditorContext;
class MidiEvent;
class MidiNoteEvent;
class MidiSong;
class MidiSequencer;
class MidiSelectionModel;
class ReplaceDataCommand;
class Scale;


using ReplaceDataCommandPtr = std::shared_ptr<ReplaceDataCommand>;
using MidiEventPtr = std::shared_ptr<MidiEvent>;
using ScalePtr = std::shared_ptr<Scale>;

class ReplaceDataCommand : public SqCommand
{
public:
    virtual void execute(MidiSequencerPtr, SequencerWidget*) override;
    virtual void undo(MidiSequencerPtr, SequencerWidget*) override;

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
    static ReplaceDataCommandPtr makeInsertNoteCommand(
        std::shared_ptr<MidiSequencer> seq,
        std::shared_ptr<const MidiNoteEvent>,
        bool extendSelection);
    static ReplaceDataCommandPtr makeChangePitchCommand(std::shared_ptr<MidiSequencer> seq, int semitones);
    static ReplaceDataCommandPtr makeChangeStartTimeCommand(std::shared_ptr<MidiSequencer> seq, float delta, float quantizeGrid);
    static ReplaceDataCommandPtr makeChangeStartTimeCommand(std::shared_ptr<MidiSequencer> seq, const std::vector<float>&);
    
    static ReplaceDataCommandPtr makeChangeDurationCommand(
        std::shared_ptr<MidiSequencer> seq,
        float delta,
        bool setDurationAbsolute);
    static ReplaceDataCommandPtr makeChangeDurationCommand(
        std::shared_ptr<MidiSequencer> seq,  
        const std::vector<float>&);
    static ReplaceDataCommandPtr makePasteCommand(std::shared_ptr<MidiSequencer> seq);

    static ReplaceDataCommandPtr makeMoveEndCommand(std::shared_ptr<MidiSequencer> seq, float newLength);

    static ReplaceDataCommandPtr makeReversePitchCommand(std::shared_ptr<MidiSequencer> seq);
    /**
     * This one works for any XFORM that does a one to one processing of notes.
     * Since it uses "Legacy" helper, the lambda must process notes in place.
     */
    using FilterFunc = std::function<void(MidiEventPtr)>;
    static ReplaceDataCommandPtr makeFilterNoteCommand(const std::string& name, std::shared_ptr<MidiSequencer> seq, FilterFunc);
        

    enum class Ornament {None, Trill, Arpeggio, Arpeggio2};
    static ReplaceDataCommandPtr makeChopNoteCommand(
        std::shared_ptr<MidiSequencer>sequencer, 
        int numNotes,
        Ornament,
        ScalePtr,
        int steps);

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

    /**
     * When we select new events, should we clear first?
     */
    bool extendSelection = false;

    /**
     * Some operations require the track length to be extended
     * to accommodate the edited notes. This function rounds to bar
     * boundaries; if the result is longer than the current track length,
     * then that will be returned. If track length already sufficient, will
     * return -1;
     */
    static float calculateDurationRequest(std::shared_ptr<MidiSequencer> seq, float duration);

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

