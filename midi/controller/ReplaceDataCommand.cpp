#include "ReplaceDataCommand.h"
#include "MidiLock.h"
#include "MidiSequencer.h"
#include "MidiSong.h"
#include "MidiTrack.h"
#include "SqClipboard.h"
#include "SqMidiEvent.h"
#include "TimeUtils.h"

#include <assert.h>

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

void ReplaceDataCommand::execute(MidiSequencerPtr seq, SequencerWidget*)
{
    assert(seq);
    MidiTrackPtr mt = seq->song->getTrack(trackNumber);
    assert(mt);
    MidiLocker l(mt->lock);

    const float currentTrackLength = mt->getLength();
    const bool isNewLengthRequested = (newTrackLength >= 0);
    const bool isNewLengthLonger = (newTrackLength > currentTrackLength);
   
    // If we need to make track longer, do it first
    if (isNewLengthRequested && isNewLengthLonger) {
        mt->setLength(newTrackLength);
    }

    for (auto it : addData) {
        mt->insertEvent(it);
    }

    for (auto it : removeData) {
        mt->deleteEvent(*it);
    }

    //  if we need to make track shorter, do it last
    if (isNewLengthRequested && !isNewLengthLonger) {
        mt->setLength(newTrackLength);
    }

    // clone the selection, clear real selection, add stuff back correctly
    // at the very least we must clear the selection, as those notes are no
    // longer in the track.

    MidiSelectionModelPtr selection = seq->selection;
    assert(selection);
   // MidiSelectionModelPtr reference = selection->clone();
   // assert(reference);

    if (!extendSelection) {
        selection->clear();
    }
    
    for (auto it : addData) {
        auto foundIter = mt->findEventDeep(*it);      // find an event in the track that matches the one we just inserted
        assert(foundIter != mt->end());
        MidiEventPtr evt = foundIter->second;
        selection->extendSelection(evt);
    }
}

void ReplaceDataCommand::undo(MidiSequencerPtr seq, SequencerWidget*)
{
    assert(seq);
    MidiTrackPtr mt = seq->song->getTrack(trackNumber);
    assert(mt);
    MidiLocker l(mt->lock);

    // we may need to change length back to originalTrackLength
    const float currentTrackLength = mt->getLength();
    const bool isNewLengthRequested = (originalTrackLength >= 0);
    const bool isNewLengthLonger = (originalTrackLength > currentTrackLength);

    // If we need to make track longer, do it first
    if (isNewLengthRequested && isNewLengthLonger) {
        mt->setLength(originalTrackLength);
    }

    // to undo the insertion, delete all of them
    for (auto it : addData) {
        mt->deleteEvent(*it);
    }
    for (auto it : removeData) {
        mt->insertEvent(it);
    }

        // If we need to make track shorter, do it last
    if (isNewLengthRequested && !isNewLengthLonger) {
        mt->setLength(originalTrackLength);
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

    float newTrackLength = -1;         // assume we won't need to change track length
    if (canChangeLength) {
        // Figure out the duration of the track after xforming the notes
        MidiSelectionModelPtr clonedSelection = seq->selection->clone();
        // find required length
        MidiEndEventPtr end = seq->context->getTrack()->getEndEvent();
        float endTime = end->startTime;

        int index = 0;  // hope index is stable across clones
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

        // now end time is the required duration
        // set up events to extend to that length
        newTrackLength = calculateDurationRequest(seq, endTime);
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
        toAdd,
        newTrackLength);
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeFilterNoteCommand(
    const std::string& name, 
    std::shared_ptr<MidiSequencer> seq, 
    FilterFunc lambda)
{
    seq->assertValid(); 
    Xform xform = [lambda](MidiEventPtr event, int) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            lambda(note);
        }
    };
    auto ret = makeChangeNoteCommand(Ops::Pitch, seq, xform, false);
    ret->name = name;
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

ReplaceDataCommandPtr ReplaceDataCommand::makeChangeStartTimeCommand(MidiSequencerPtr seq, float delta, float quantizeGrid)
{
    seq->assertValid();
    Xform xform = [delta, quantizeGrid](MidiEventPtr event, int) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            float s = note->startTime;
            s += delta;
            s = std::max(0.f, s);
            if (quantizeGrid != 0) {
                s = (float) TimeUtils::quantize(s, quantizeGrid, true);
            }
            note->startTime = s;
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

ReplaceDataCommandPtr ReplaceDataCommand::makeChangeDurationCommand(MidiSequencerPtr seq, float delta, bool setDurationAbsolute)
{
    seq->assertValid();
    Xform xform = [delta, setDurationAbsolute](MidiEventPtr event, int) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            if (setDurationAbsolute) {
                assert(delta > .001f);
                note->duration = delta;
            } else {
                note->duration += delta;
                 // arbitrary min limit.
                note->duration = std::max(.001f, note->duration);
            }
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
    const float newTrackLength = calculateDurationRequest(seq, newDuration);
    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
        seq->context,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd,
        newTrackLength);
    ret->name = "paste";
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeInsertNoteCommand(
    MidiSequencerPtr seq,
    MidiNoteEventPtrC origNote,
    bool extendSelection)
{
   // assert(!extendSelection);
    seq->assertValid();
    MidiNoteEventPtr note = origNote->clonen();

    const float newDuration = calculateDurationRequest(seq, note->startTime + note->duration);
  
    // Make the delete end / inserts end to extend track.
    // Make it long enough to hold insert note.
    std::vector<MidiEventPtr> toRemove;
    std::vector<MidiEventPtr> toAdd;

    toAdd.push_back(note);

    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
        seq->context,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd,
        newDuration);
    ret->name = "insert note";
    ret->extendSelection = extendSelection;
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeMoveEndCommand(std::shared_ptr<MidiSequencer> seq, float newLength)
{
    seq->assertValid();

    std::vector<MidiEventPtr> toAdd;
    std::vector<MidiEventPtr> toDelete;

    modifyNotesToFitNewLength(seq, newLength, toAdd, toDelete);
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

void ReplaceDataCommand::modifyNotesToFitNewLength(
    std::shared_ptr<MidiSequencer>seq,
    float newLength,
    std::vector<MidiEventPtr>& toAdd,
    std::vector<MidiEventPtr>& toDelete)
{
    auto tk = seq->context->getTrack();
    for (auto it : *tk) {
        MidiEventPtr ev = it.second;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);
        if (note) {
            if (note->startTime >= newLength) {
                toDelete.push_back(note);
            } else if (note->endTime() > newLength) {
                MidiNoteEventPtr newNote = safe_cast<MidiNoteEvent>(note->clone());
                toDelete.push_back(note);
                newNote->duration = newLength - newNote->startTime;
                toAdd.push_back(newNote);
            }
        }
    }
}

float ReplaceDataCommand::calculateDurationRequest(MidiSequencerPtr seq, float duration)
{
    const float currentDuration = seq->context->getTrack()->getLength();
    if (currentDuration >= duration) {
        return -1;                      // Don't need to do anything, long enough
    }

    const float needBars = duration / 4.f;
    const float roundedBars = std::floor(needBars + 1.f);
    const float durationRequest = roundedBars * 4;
    return durationRequest;
}


// Algorithm

// clone selection -> clone

// enumerate clone, flip the pitches, using selection is ref
// to add = clone (as vector)
// to remove = selection (as vector)

ReplaceDataCommandPtr ReplaceDataCommand::makeReversePitchCommand(std::shared_ptr<MidiSequencer> seq)
{
    std::vector<MidiEventPtr> toRemove;
    std::vector<MidiEventPtr> toAdd;

    // will transform the cloned selection, and add it
    auto clonedSelection = seq->selection->clone();
    MidiSelectionModel::const_reverse_iterator itDest = clonedSelection->rbegin();

    for (MidiSelectionModel::const_iterator itSrc = seq->selection->begin(); itSrc != seq->selection->end(); ++itSrc) {
        MidiEventPtr srcEvent = *itSrc;
        MidiEventPtr destEvent = *itDest;

        MidiNoteEventPtr destNote = safe_cast<MidiNoteEvent>(destEvent);
        MidiNoteEventPtr srcNote = safe_cast<MidiNoteEvent>(srcEvent);

        if (srcNote) {
            assert(destNote);
            destNote->pitchCV = srcNote->pitchCV;
        }
        ++itDest;
    }

    // we will remove all the events in the selection
    toRemove = seq->selection->asVector();
    toAdd = clonedSelection->asVector();

    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
        seq->context,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    ret->name = "reverse pitches";
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeChopNoteCommand(
    std::shared_ptr<MidiSequencer> seq, 
    int numNotes,
    Ornament ornament,
    ScalePtr scale,
    int steps)
{
    assert(ornament == Ornament::None);
    assert(!scale);
    assert(steps == 0);
    std::vector<MidiEventPtr> toRemove;
    std::vector<MidiEventPtr> toAdd;

    // toAdd will get the new notes derived from chopping.
    for (MidiSelectionModel::const_iterator it = seq->selection->begin(); it != seq->selection->end(); ++it) {
        MidiEventPtr event = *it;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
       
        if (note) {
            const float dur = note->duration;
            const float durTotal = TimeUtils::getTimeAsPowerOfTwo16th(dur);
            if (durTotal > 0) {
                for (int i = 0; i < numNotes; ++i) {
                    MidiNoteEventPtr newNote = std::make_shared<MidiNoteEvent>();
                    newNote->startTime = note->startTime + i * durTotal / numNotes;
                    newNote->duration = dur / numNotes;     // keep original articulation
                    newNote->pitchCV = note->pitchCV;
                    toAdd.push_back(newNote);
                }
                toRemove.push_back(note);
            }
        }
    }

    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
        seq->context,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    ret->name = "chop notes";
    return ret;
}
