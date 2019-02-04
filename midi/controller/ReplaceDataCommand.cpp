
#include <assert.h>
#include "ReplaceDataCommand.h"
#include "MidiTrack.h"
#include "MidiSequencer.h"
#include "MidiSong.h"

ReplaceDataCommand::ReplaceDataCommand(
    MidiSongPtr song,
    MidiSelectionModelPtr selection,
    int trackNumber,
    const std::vector<MidiEventPtr>& inRemove,
    const std::vector<MidiEventPtr>& inAdd)
    : song(song), trackNumber(trackNumber), selection(selection), removeData(inRemove), addData(inAdd)
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

    // clone the selection, clear real selection, add stuff back correctly
    // at the very least we must clear the selection, as those notes are no
    // longer in the track.
    MidiSelectionModelPtr reference = selection->clone();
    selection->clear();
    for (auto it : addData) {
        auto foundIter = mt->findEventDeep(*it);      // find an event in the track that matches the one we just inserted
        assert(foundIter != mt->end());
        MidiEventPtr evt = foundIter->second;
        selection->extendSelection(evt);
    }
#if 0 // first try, failed
    for (auto it : *reference) {
        auto foundIter = mt->findEventDeep(*it);      // find an event in the track that matches the old one
        assert(foundIter != mt->end());
        MidiEventPtr evt = foundIter->second;
        selection->extendSelection(evt);
    }
#endif
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

    selection->clear();
    for (auto it : removeData) {
        auto foundIter = mt->findEventDeep(*it);      // find an event in the track that matches the one we just inserted
        assert(foundIter != mt->end());
        MidiEventPtr evt = foundIter->second;
        selection->extendSelection(evt);
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
        seq->selection,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeChangePitchCommand(MidiSequencerPtr seq, int semitones)
{
    const float deltaCV = PitchUtils::semitone * semitones;
    MidiSelectionModelPtr clonedSelection = seq->selection->clone();

    // will remove existing selection
    std::vector<MidiEventPtr> toRemove;
    for (auto it : *seq->selection) {
        auto note = safe_cast<MidiNoteEvent>(it);
        if (note) {
            toRemove.push_back(note);
        }
    }

    // and add back the transposed notes
    std::vector<MidiEventPtr> toAdd;
    for (auto it : *clonedSelection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it);
        if (note) {
            note->pitchCV += deltaCV;
            toAdd.push_back(note);
        }
    }

    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
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

    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    return ret;
}