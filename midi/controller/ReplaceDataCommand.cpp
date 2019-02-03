
#include <assert.h>
#include "ReplaceDataCommand.h"
#include "MidiTrack.h"
#include "MidiSequencer.h"
#include "MidiSong.h"

ReplaceDataCommand::ReplaceDataCommand(
    std::shared_ptr<MidiSong> song,
    int trackNumber,
    const std::vector<MidiEventPtr>& inRemove,
    const std::vector<MidiEventPtr>& inAdd)
    : song(song), trackNumber(trackNumber), removeData(inRemove), addData(inAdd)
{
    assert(song->getTrack(trackNumber));
}

void ReplaceDataCommand::execute()
{
    MidiTrackPtr mt = song->getTrack(trackNumber);
    assert(mt);

    for (auto it : addData) {
        mt->insertEvent(it);
    }

    for (auto it : removeData) {
        mt->deleteEvent(*it);
    }
}

void ReplaceDataCommand::undo()
{
    MidiTrackPtr mt = song->getTrack(trackNumber);
    assert(mt);

    // to undo the insertion, delete all of them
    for (auto it : addData) {
        mt->deleteEvent(*it);
    }
    for (auto it : removeData) {
        mt->insertEvent(it);
    }
}


ReplaceDataCommandPtr ReplaceDataCommand::makeDeleteCommand(MidiSequencerPtr seq)
{
    std::vector<MidiEventPtr> toRemove;
    std::vector<MidiEventPtr> toAdd;
    auto track = seq->context->getTrack();
    for (auto it : *seq->selection) {
        MidiEventPtr ev = it;
        toRemove.push_back(ev);
    }
    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeChangePitchCommand(MidiSequencerPtr seq, int semitones)
{
    assert(false);      // doesn't do anything.

    // will remove existing selection
    std::vector<MidiEventPtr> toRemove;

    // and add back the tranposed notes
    std::vector<MidiEventPtr> toAdd;
    auto track = seq->context->getTrack();

    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeInsertNoteCommand(MidiSequencerPtr seq)
{
    assert(false);      // doesn't do anything.
    std::vector<MidiEventPtr> toRemove;
    std::vector<MidiEventPtr> toAdd;
    auto track = seq->context->getTrack();

    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    return ret;
}