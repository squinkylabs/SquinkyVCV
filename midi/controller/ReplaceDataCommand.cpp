#include "ReplaceDataCommand.h"
#include "MidiLock.h"
#include "MidiSequencer.h"
#include "MidiSong.h"
#include "MidiTrack.h"
#include "SqClipboard.h"

#include <assert.h>

#if 0 // old way with persistent song pointers
ReplaceDataCommand::ReplaceDataCommand(
    MidiSongPtr song,
    MidiSelectionModelPtr selection,
    std::shared_ptr<MidiEditorContext> unused,
    int trackNumber,
    const std::vector<MidiEventPtr>& inRemove,
    const std::vector<MidiEventPtr>& inAdd)
    : song(song), trackNumber(trackNumber), selection(selection), removeData(inRemove), addData(inAdd)
{
    assert(song->getTrack(trackNumber));
}
#endif


ReplaceDataCommand::ReplaceDataCommand(
    MidiSongPtr song,
    MidiSelectionModelPtr selection,
    std::shared_ptr<MidiEditorContext> unused,
    int trackNumber,
    const std::vector<MidiEventPtr>& inRemove,
    const std::vector<MidiEventPtr>& inAdd,
    float trackLength)
    : trackNumber(trackNumber), removeData(inRemove), addData(inAdd), newTrackLength(trackLength)
{
    assert(song->getTrack(trackNumber));
    originalTrackLength = song->getTrack(trackNumber)->getLength();     /// save off
}


ReplaceDataCommand::ReplaceDataCommand(
    MidiSongPtr song,
    int trackNumber,
    const std::vector<MidiEventPtr>& inRemove,
    const std::vector<MidiEventPtr>& inAdd)
    : trackNumber(trackNumber), removeData(inRemove), addData(inAdd)
{
    assert(song->getTrack(trackNumber));
}


void ReplaceDataCommand::execute(MidiSequencerPtr seq)
{
    assert(seq);
    MidiTrackPtr mt = seq->song->getTrack(trackNumber);
    assert(mt);
    MidiLocker l(mt->lock);

    for (auto it : addData) {
        mt->insertEvent(it);
    }

    for (auto it : removeData) {
        mt->deleteEvent(*it);
    }

    // last, adjust the duration, if required
    // note that this is way to simplified for real life!
    if (newTrackLength >= 0) {
        auto xx = mt->getEndEvent();
        MidiEndEventPtr end = mt->getEndEvent();
        assert(end);
        mt->deleteEvent(*end);
        mt->insertEnd(newTrackLength);
        mt->assertValid();
    }

    // clone the selection, clear real selection, add stuff back correctly
    // at the very least we must clear the selection, as those notes are no
    // longer in the track.

    MidiSelectionModelPtr selection = seq->selection;
    assert(selection);
    MidiSelectionModelPtr reference = selection->clone();
    assert(reference);
    selection->clear();
    for (auto it : addData) {
        auto foundIter = mt->findEventDeep(*it);      // find an event in the track that matches the one we just inserted
        assert(foundIter != mt->end());
        MidiEventPtr evt = foundIter->second;
        selection->extendSelection(evt);
    }
}

void ReplaceDataCommand::undo(MidiSequencerPtr seq)
{
    assert(seq);
    MidiTrackPtr mt = seq->song->getTrack(trackNumber);
    assert(mt);
    MidiLocker l(mt->lock);

    // first, reset the length
    if (newTrackLength >= 0) {
        auto xx = mt->getEndEvent();
        MidiEndEventPtr end = mt->getEndEvent();
        assert(end);
        mt->deleteEvent(*end);
        mt->insertEnd(originalTrackLength);
        mt->assertValid();
    }

    // to undo the insertion, delete all of them
    for (auto it : addData) {
        mt->deleteEvent(*it);
    }
    for (auto it : removeData) {
        mt->insertEvent(it);
    }

    MidiSelectionModelPtr selection = seq->selection;
    assert(selection);
    selection->clear();
    for (auto it : removeData) {
        auto foundIter = mt->findEventDeep(*it);      // find an event in the track that matches the one we just inserted
        assert(foundIter != mt->end());
        MidiEventPtr evt = foundIter->second;
        selection->extendSelection(evt);
    }
    // TODO: move cursor
}

ReplaceDataCommandPtr ReplaceDataCommand::makeDeleteCommand(MidiSequencerPtr seq, const char* name)
{
    seq->assertValid();
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
        seq->context,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);

    ret->name = name;
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeChangeNoteCommand(
    Ops op,
    std::shared_ptr<MidiSequencer> seq,
    Xform xform,
    bool canChangeLength)
{
    seq->assertValid();

    std::vector<MidiEventPtr> toAdd;
    std::vector<MidiEventPtr> toRemove;

    if (canChangeLength) {
        // Figure out the duration of the track after xforming the notes
        MidiSelectionModelPtr clonedSelection = seq->selection->clone();
        // find required length
        MidiEndEventPtr end = seq->context->getTrack()->getEndEvent();
        float endTime = end->startTime;

        int index = 0;  // hope index is stable accross clones
        for (auto it : *clonedSelection) {
            MidiEventPtr ev = it;
            xform(ev, index++);
            float t = ev->startTime;
            MidiNoteEventPtrC note = safe_cast<MidiNoteEvent>(ev);
            if (note) {
                t += note->duration;
            }
            endTime = std::max(endTime, t);
        }
        // set up events to extend to that length
        if (endTime > end->startTime) {
            extendTrackToMinDuration(seq, endTime, toAdd, toRemove);
        }
    }

    MidiSelectionModelPtr clonedSelection = seq->selection->clone();

    // will remove existing selection
    for (auto it : *seq->selection) {
        auto note = safe_cast<MidiNoteEvent>(it);
        if (note) {
            toRemove.push_back(note);
        }
    }

    // and add back the transformed notes
    int index=0;
    for (auto it : *clonedSelection) {
        MidiEventPtr event = it;
        xform(event, index++);
        toAdd.push_back(event);
    }

    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
        seq->context,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeChangePitchCommand(MidiSequencerPtr seq, int semitones)
{
    seq->assertValid();
    const float deltaCV = PitchUtils::semitone * semitones;
    Xform xform = [deltaCV](MidiEventPtr event, int) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            float newPitch = note->pitchCV + deltaCV;
            newPitch = std::min(10.f, newPitch);
            newPitch = std::max(-10.f, newPitch);
            note->pitchCV = newPitch;
        }
    };
    auto ret = makeChangeNoteCommand(Ops::Pitch, seq, xform, false);
    ret->name = "change pitch";
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeChangeStartTimeCommand(MidiSequencerPtr seq, float delta)
{
    seq->assertValid();
    Xform xform = [delta](MidiEventPtr event, int) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            note->startTime += delta;
            note->startTime = std::max(0.f, note->startTime);
        }
    };
    auto ret =  makeChangeNoteCommand(Ops::Start, seq, xform, true);
    ret->name = "change note start";
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeChangeStartTimeCommand(MidiSequencerPtr seq, const std::vector<float>& shifts)
{
    seq->assertValid();
    Xform xform = [shifts](MidiEventPtr event, int index) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            note->startTime += shifts[index];
            note->startTime = std::max(0.f, note->startTime);
        }
    };
    auto ret =  makeChangeNoteCommand(Ops::Start, seq, xform, true);
    ret->name = "change note start";
    return ret;
}


ReplaceDataCommandPtr ReplaceDataCommand::makeChangeDurationCommand(MidiSequencerPtr seq, float delta)
{
    seq->assertValid();
    Xform xform = [delta](MidiEventPtr event, int) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            note->duration += delta;
             // arbitrary min limit.
            note->duration = std::max(.001f, note->duration);
        }
    };
    auto ret = makeChangeNoteCommand(Ops::Duration, seq, xform, true);
    ret->name = "change note duration";
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeChangeDurationCommand(MidiSequencerPtr seq, const std::vector<float>& shifts)
{
    seq->assertValid();
    Xform xform = [shifts](MidiEventPtr event, int index) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            note->duration += shifts[index];
             // arbitrary min limit.
            note->duration = std::max(.001f, note->duration);
        }
    };
    auto ret = makeChangeNoteCommand(Ops::Duration, seq, xform, true);
    ret->name = "change note duration";
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makePasteCommand(MidiSequencerPtr seq)
{
    seq->assertValid();
    std::vector<MidiEventPtr> toAdd;
    std::vector<MidiEventPtr> toRemove;

    auto clipData = SqClipboard::getTrackData();
    assert(clipData);

    // all the selected notes get deleted
    for (auto it : *seq->selection) {
        toRemove.push_back(it);
    }

    const float insertTime = seq->context->cursorTime();
    const float eventOffsetTime = insertTime - clipData->offset;

    // copy all the notes on the clipboard into the track, but move to insert time

    float newDuration = 0;
    for (auto it : *clipData->track) {
        MidiEventPtr evt = it.second->clone();
        evt->startTime += eventOffsetTime;
        assert(evt->startTime >= 0);
        if (evt->type != MidiEvent::Type::End) {
            toAdd.push_back(evt);
            newDuration = std::max(newDuration, evt->startTime);
        }
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(evt);
        if (note) {
            newDuration = std::max(newDuration, note->duration + note->startTime);
        }
    }
    extendTrackToMinDuration(seq, newDuration, toAdd, toRemove);

   // printf("Make paste command, add %d, remove %d\n", (int) toAdd.size(), (int) toRemove.size());
    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
        seq->context,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    ret->name = "paste";
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeInsertNoteCommand(MidiSequencerPtr seq, MidiNoteEventPtrC origNote)
{
    seq->assertValid();
    MidiNoteEventPtr note = origNote->clonen();

    // Make the delete end / inserts end to extend track.
    // Make it long enough to hold insert note.
    std::vector<MidiEventPtr> toRemove;
    std::vector<MidiEventPtr> toAdd;
    extendTrackToMinDuration(seq, note->startTime + note->duration, toAdd, toRemove);

    toAdd.push_back(note);

    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
        seq->context,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    ret->name = "insert note";
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeMoveEndCommand(std::shared_ptr<MidiSequencer> seq, float newLength)
{
    seq->assertValid();

    std::vector<MidiEventPtr> toAdd;
    std::vector<MidiEventPtr> toDelete;
    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
        seq->context,
        seq->context->getTrackNumber(),
        toDelete,
        toAdd,
        newLength);
    ret->name = "move end point";
    return ret;
}

void ReplaceDataCommand::extendTrackToMinDuration(
    MidiSequencerPtr seq,
    float neededLength,
    std::vector<MidiEventPtr>& toAdd,
    std::vector<MidiEventPtr>& toDelete)
{
    auto track = seq->context->getTrack();
    float curLength = track->getLength();

    if (neededLength > curLength) {
        float need = neededLength;
        float needBars = need / 4.f;
        float roundedBars = std::round(needBars + 1.f);
        float duration = roundedBars * 4;
        std::shared_ptr<MidiEndEvent> end = track->getEndEvent();
        track->deleteEvent(*end);
        track->insertEnd(duration);
    }
}