
#include <assert.h>
#include "MidiEditor.h"
#include "MidiEditorContext.h"
#include "MidiLock.h"
#include "MidiSelectionModel.h"
#include "MidiSequencer.h"
#include "MidiSong.h"
#include "MidiTrack.h"
#include "ReplaceDataCommand.h"
#include "SqClipboard.h"
#include "TimeUtils.h"

extern int _mdb;

MidiEditor::MidiEditor(std::shared_ptr<MidiSequencer> seq) :
    m_seq(seq)
{
    _mdb++;
}

MidiEditor::~MidiEditor()
{
    _mdb--;
}

MidiTrackPtr MidiEditor::getTrack()
{
    return seq()->song->getTrack(seq()->context->getTrackNumber());
}

/**
 * returns the not that was added to selection, or nullptr if none
 */
static MidiNoteEventPtr selectNextNotePastCursor(bool atCursorOk,
    bool keepExisting,
    MidiSequencerPtr seq)
{
    const auto t = seq->context->cursorTime();
    const auto track = seq->context->getTrack();

    // first, seek into track until cursor time.
    MidiTrack::const_iterator it = track->seekToTimeNote(t);
    if (it == track->end()) {
        if (!keepExisting) {
            seq->selection->clear();
        }
        return nullptr;
    }

    // if it came back with a note exactly at cursor time,
    // check if it's acceptable.
    if ((it->first > t) || (it->first == t && atCursorOk)) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it->second);
        if (note) {
            seq->selection->addToSelection(note, keepExisting);
            return note;
        }
    }

    MidiTrack::const_iterator bestSoFar = it;

    // If must be past cursor time, advance to next
    ++it;

    for (; it != track->end(); ++it) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it->second);
        if (note) {
            seq->selection->addToSelection(note, keepExisting);
            return note;
        }
    }

    // If nothing past where we are, it's ok, even if it is at the same time
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(bestSoFar->second);
    if (note) {
        seq->selection->addToSelection(note, keepExisting);
    }
    return note;
}

static MidiNoteEventPtr selectPrevNoteBeforeCursor(bool atCursorOk,
    bool keepExisting,
    MidiSequencerPtr seq)
{
    const auto t = seq->context->cursorTime();
    const auto track = seq->context->getTrack();

    // first, seek into track until cursor time.
    MidiTrack::const_iterator it = track->seekToTimeNote(t);
    if (it == track->end()) {
        it = track->seekToLastNote();
        if (it == track->end()) {
            seq->selection->clear();
            return nullptr;
        }
    }

    // if it came back with a note exactly at cursor time,
    // check if it's acceptable.
    if ((it->first < t) || (it->first == t && atCursorOk)) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it->second);
        if (note) {
            seq->selection->addToSelection(note, keepExisting);
            return note;
        }
    }


    MidiTrack::const_iterator bestSoFar = it;
    // If must be before cursor time, go back to prev

  //  if (it != track->begin()) {
  //      --it;
  //  }

    // now either this previous is acceptable, or something before it
    while (true) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it->second);
        if (note && (note->startTime < t)) {
            seq->selection->addToSelection(note, keepExisting);
            return note;
        }
        if (it == track->begin()) {
            break;  // give up if we are at start and have found nothing good
        }
        --it;       // if nothing good, try previous
    }

    // If nothing past where we are, it's OK, even if it is at the same time
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(bestSoFar->second);
    if (note && note->startTime >= t) {
        note = nullptr;
    }
    if (note) {
        seq->selection->select(bestSoFar->second);
    }
    return note;
}

/**
 * Here's the "new" algorithm:
 *  if there is a note selected, search for the next note that's later
 *  in the track than the current selection. If not found,
 *  don't change anything.
 *
 *  If nothing selected, then search for the first note that is later or at the same time as the cursor.
 *
 * For now we can base everything from cursor. Later, when we do multi-select, will need to be smarter.
 */

void MidiEditor::selectNextNote()
{
    seq()->assertValid();

    MidiTrackPtr track = getTrack();
    assert(track);
    const bool acceptCursorTime = seq()->selection->empty();
    selectNextNotePastCursor(acceptCursorTime, false, seq());

    updateCursor();
    seq()->context->adjustViewportForCursor();
}

void MidiEditor::extendSelectionToNextNote()
{
    seq()->assertValid();

    MidiTrackPtr track = getTrack();
    assert(track);
    const bool acceptCursorTime = seq()->selection->empty();
    MidiNoteEventPtr note =  selectNextNotePastCursor(acceptCursorTime, true, seq());

    // move cursor to newly selected note - it if exists
    if (note) {
        setCursorToNote(note);
    } else {
        updateCursor();
    }
    seq()->context->adjustViewportForCursor();
}

void MidiEditor::selectPrevNote()
{
    seq()->assertValid();
    MidiTrackPtr track = getTrack();
    assert(track);
    const bool acceptCursorTime = seq()->selection->empty();
    selectPrevNoteBeforeCursor(acceptCursorTime, false, seq());
    updateCursor();
    seq()->context->adjustViewportForCursor();
}

void MidiEditor::extendSelectionToPrevNote()
{
    seq()->assertValid();

    MidiTrackPtr track = getTrack();
    assert(track);
    const bool acceptCursorTime = seq()->selection->empty();
    MidiNoteEventPtr note = selectPrevNoteBeforeCursor(acceptCursorTime, true, seq());
      // move cursor to newly selected note - it if exists
    if (note) {
        setCursorToNote(note);
    } else {
        updateCursor();
    }
    seq()->context->adjustViewportForCursor();
}


void MidiEditor::setCursorToNote(MidiNoteEventPtr note)
{
    seq()->context->setCursorTime(note->startTime);
    seq()->context->setCursorPitch(note->pitchCV);
}

void MidiEditor::updateCursor()
{
    if (seq()->selection->empty()) {
        return;
    }

    MidiNoteEventPtr firstNote;
    // If cursor is already in selection, leave it there.
    for (auto it : *seq()->selection) {
        MidiEventPtr ev = it;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);
        if (note) {
            if (!firstNote) {
                firstNote = note;
            }
            if ((note->startTime == seq()->context->cursorTime()) &&
                (note->pitchCV == seq()->context->cursorPitch())) {
                return;
            }
        }
    }
    setCursorToNote(firstNote);
}


void MidiEditor::changePitch(int semitones)
{
   
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangePitchCommand(seq(), semitones);
    seq()->undo->execute(seq(), cmd);
    seq()->assertValid();
    float deltaCV = PitchUtils::semitone * semitones;


    // Now fix-up selection and view-port
    float newCursorPitch = seq()->context->cursorPitch() + deltaCV;
    newCursorPitch = std::min(10.f, newCursorPitch);
    newCursorPitch = std::max(-10.f, newCursorPitch);

    printf("changePitch newcv = %f\n", newCursorPitch); fflush(stdout);

    seq()->context->setCursorPitch(newCursorPitch);
    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
}

void MidiEditor::changeStartTime(bool ticks, int amount)
{
    MidiLocker l(seq()->song->lock);
    assert(amount != 0);

    // "units" are 16th, "ticks" are 64th
    float advanceAmount = amount * (ticks ? (1.f / 16.f) : (1.f / 4.f));


    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangeStartTimeCommand(seq(), advanceAmount);
    seq()->undo->execute(seq(), cmd);
    seq()->assertValid();

    // after we change start times, we need to put the cursor on the moved notes
    seq()->context->setCursorToSelection(seq()->selection);

    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
}

void MidiEditor::changeDuration(bool ticks, int amount)
{
    assert(amount != 0);

    float advanceAmount = amount * (ticks ? (1.f / 16.f) : (1.f / 4.f));

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangeDurationCommand(seq(), advanceAmount);
    seq()->undo->execute(seq(), cmd);
    seq()->assertValid();
}

void MidiEditor::assertCursorInSelection()
{
    bool foundIt = false;
    (void) foundIt;
    assert(!seq()->selection->empty());
    for (auto it : *seq()->selection) {
        if (seq()->context->cursorTime() == it->startTime) {
            foundIt = true;
        }
    }
    assert(foundIt);
}

void MidiEditor::advanceCursorToTime(float time)
{
   // assert(!ticks);         // not implemented yet
//assert(amount != 0);

  //  seq()->context->assertCursorInViewport();

  //  float advanceAmount = amount * 1.f / 4.f;       // hard code units to 1/16th notes
  //  seq()->context->setCursorTime(seq()->context->cursorTime() + advanceAmount);
    seq()->context->setCursorTime(std::max(0.f, time));
    updateSelectionForCursor();
    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
    seq()->assertValid();
}

void MidiEditor::advanceCursor(bool ticks, int amount)
{
    assert(!ticks);         // not implemented yet
    assert(amount != 0);

    seq()->context->assertCursorInViewport();

    float advanceAmount = amount * 1.f / 4.f;       // hard code units to 1/16th notes
    float newTime = seq()->context->cursorTime() + advanceAmount;
    advanceCursorToTime(newTime);
}

#if 0
void MidiEditor::advanceCursor(bool ticks, int amount)
{
    assert(!ticks);         // not implemented yet
    assert(amount != 0);

    seq()->context->assertCursorInViewport();

    float advanceAmount = amount * 1.f / 4.f;       // hard code units to 1/16th notes
    seq()->context->setCursorTime(seq()->context->cursorTime() + advanceAmount);
    seq()->context->setCursorTime(std::max(0.f, seq()->context->cursorTime()));
    updateSelectionForCursor();
    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
    seq()->assertValid();
}
#endif

void MidiEditor::changeCursorPitch(int semitones)
{
    float pitch = seq()->context->cursorPitch() + (semitones * PitchUtils::semitone);
    setNewCursorPitch(pitch);
    #if 0
    pitch = std::max(pitch, -5.f);
    pitch = std::min(pitch, 5.f);
    seq()->context->setCursorPitch(pitch);
    seq()->context->scrollViewportToCursorPitch();
    updateSelectionForCursor();
    #endif
}

void MidiEditor::setNewCursorPitch(float pitch)
{
    pitch = std::max(pitch, -5.f);
    pitch = std::min(pitch, 5.f);
    seq()->context->setCursorPitch(pitch);
    seq()->context->scrollViewportToCursorPitch();
    updateSelectionForCursor();
}

void MidiEditor::selectAt(float time, float pitchCV)
{
    // Implement by calling existing handlers. This will
    // cause double update, but I don't think anyone cares.
    setNewCursorPitch(pitchCV);
    advanceCursorToTime(time);
}

void MidiEditor::extendTrackToMinDuration(float neededLength)
{
    auto track = seq()->context->getTrack();
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

void MidiEditor::insertNoteHelper(Durations dur, bool moveCursorAfter)
{
    MidiLocker l(seq()->song->lock);
    const float artic = 7.f/8.f;
    float duration = 1;
    float cursorAdvance = 0;
    switch (dur) {
        case Durations::Whole:
            cursorAdvance = moveCursorAfter ? 4.f : 0;
            duration = 4.f * artic;
            break;
        case Durations::Half:
            cursorAdvance = moveCursorAfter ? 2.f : 0;
            duration = 2.f * artic;
            break;
        case Durations::Quarter:
            cursorAdvance = moveCursorAfter ? 1.f : 0;
            duration = 1.f * artic;
            break;
        case Durations::Eighth:
            cursorAdvance = moveCursorAfter ? .5f : 0;
            duration = .5f * artic;
            break;
        case Durations::Sixteenth:
            cursorAdvance = moveCursorAfter ? .25f : 0;
            duration = .25f * artic;
            break;
        default:
            assert(false);
    }

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = seq()->context->cursorTime();
    note->pitchCV = seq()->context->cursorPitch();
    note->duration = duration;
    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq(), note);

    seq()->undo->execute(seq(), cmd);
    seq()->context->setCursorTime(note->startTime + cursorAdvance);

    updateSelectionForCursor();
    seq()->assertValid();
}

void MidiEditor::insertPresetNote(Durations dur)
{
    insertNoteHelper(dur, true);
}

void MidiEditor::insertNote()
{
    insertNoteHelper(Durations::Quarter, false);
    #if 0
    MidiLocker l(seq()->song->lock);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = seq()->context->cursorTime();
    note->pitchCV = seq()->context->cursorPitch();
    note->duration = 1;  // for now, fixed to quarter
    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq(), note);

    seq()->undo->execute(cmd);

    updateSelectionForCursor();
    seq()->assertValid();
    #endif
}

void MidiEditor::deleteNote()
{
    if (seq()->selection->empty()) {
        return;
    }

    auto cmd = ReplaceDataCommand::makeDeleteCommand(seq());

    seq()->undo->execute(seq(), cmd);
    // TODO: move selection into undo
    seq()->selection->clear();
}

void MidiEditor::updateSelectionForCursor()
{
    seq()->selection->clear();
    const int cursorSemi = PitchUtils::cvToSemitone(seq()->context->cursorPitch());

    // iterator over all the notes that are in the edit context
    auto start = seq()->context->startTime();
    auto end = seq()->context->endTime();
    MidiTrack::note_iterator_pair notes = getTrack()->timeRangeNotes(start, end);
    for (auto it = notes.first; it != notes.second; ++it) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it->second);
        const auto startTime = note->startTime;
        const auto endTime = note->startTime + note->duration;

        if ((PitchUtils::cvToSemitone(note->pitchCV) == cursorSemi) &&
            (startTime <= seq()->context->cursorTime()) &&
            (endTime > seq()->context->cursorTime())) {
            seq()->selection->select(note);
            return;
        }
    }
}

void MidiEditor::setNoteEditorAttribute(MidiEditorContext::NoteAttribute attr)
{
    seq()->context->noteAttribute = attr;
}

void MidiEditor::selectAll()
{
    seq()->selection->clear();
    MidiTrackPtr track = seq()->context->getTrack();
    for (auto it : *track) {
        MidiEventPtr orig = it.second;
        if (orig->type != MidiEvent::Type::End) {
            seq()->selection->extendSelection(orig);
        }
    }
}

void MidiEditor::cut()
{
}

void MidiEditor::copy()
{
    auto songLock = seq()->song->lock;
    MidiLocker l(songLock);

    float earliestEventTime = 0;
    bool firstOne = true;
   
    // put cloned selection into a track
    // TODO: why do we clone all the time? aren't events immutable?
    MidiTrackPtr track = std::make_shared<MidiTrack>(songLock);
    for (auto it : *seq()->selection) {
        MidiEventPtr orig = it;
        MidiEventPtr newEvent = orig->clone();
        track->insertEvent(newEvent);
        if (firstOne) {
            earliestEventTime = newEvent->startTime;
        }
        earliestEventTime = std::min(earliestEventTime, newEvent->startTime);
        firstOne = false;
    }

    if (track->size() == 0) {
        return;
    }
    // TODO: make helper? Adding a final end event
    auto it = track->end();
    --it;
    MidiEventPtr lastEvent = it->second;
    float lastT = lastEvent->startTime;
    MidiNoteEventPtr lastNote = safe_cast<MidiNoteEvent>(lastEvent);
    if (lastNote) {
        lastT += lastNote->duration;
    }

    track->insertEnd(lastT);
    track->assertValid();
    
    std::shared_ptr<SqClipboard::Track> clipData = std::make_shared< SqClipboard::Track>();
    clipData->track = track;

    auto firstNote = track->getFirstNote();
    if (!firstNote) {
        return;             // this won't work if we put non-note data in here.
    }
   // int t = TimeUtils::time2bar(firstNote->startTime);
    clipData->offset = float(earliestEventTime);
    SqClipboard::putTrackData(clipData);
}

void MidiEditor::paste()
{
    if (!SqClipboard::getTrackData()) {
        return;
    }
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makePasteCommand(seq());
    seq()->undo->execute(seq(), cmd);

    // Am finding that after this cursor pitch is not in view-port
    updateCursor();  
    seq()->context->adjustViewportForCursor();
    seq()->assertValid();

    // TODO: what do we select afterwards?
}