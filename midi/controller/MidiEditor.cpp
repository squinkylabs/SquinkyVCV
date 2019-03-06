
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
 * If iterator already points to a note, return it.
 * Otherwise search for next one
 */
#if 0
static MidiTrack::const_iterator findNextNoteOrCurrent(
    MidiTrackPtr track,
    MidiTrack::const_iterator it)
{
    if (it == track->end()) {
        return it;                  // if we are at the end, give up
    }
    for (bool done = false; !done; ) {

        if (it == track->end()) {
            done = true;
        }
        MidiEventPtr evt = it->second;
        if (evt->type == MidiEvent::Type::Note) {
            done = true;
        } else if (evt->type == MidiEvent::Type::End) {
            done = true;
        } else {
            assert(false);
            ++it;
        }
    }
    return it;
}

/**
 * Starts searching from iterator it.
 * Returns iterator to the first Note at or after iter.
 *
 * Returns track.end if can't find a note
 */
static MidiTrack::const_iterator findPrevNoteOrCurrent(
    MidiTrackPtr track,
    MidiTrack::const_iterator it)
{
    for (bool done = false; !done; ) {

        MidiEventPtr evt = it->second;
        switch (evt->type) {
            case  MidiEvent::Type::Note:
                done = true;                    // if we are on a note, then we can accept that
                break;
            case MidiEvent::Type::End:
                if (it == track->begin()) {
                    return track->end();            // Empty track, can't dec end ptr, so return "fail"
                } else {
                    --it;                           // try prev (Why???)
                }
                break;
            default:
                assert(false);
                if (it == track->begin()) {
                    return track->end();            // Empty track, can't dec end ptr, so return "fail"
                } else {
                    --it;                           // try prev
                }
        }
    }
    return it;
}
#endif


/**
 * Starts searching from iterator it.
 * Returns iterator to the first Note at or after iter,
 * and resets selection to point to that note.
 *
 * Returns track.end if can't find a note
 */
#if 0
static void selectNextNoteOrCurrent(
    MidiTrackPtr track,
    MidiTrack::const_iterator it,
    MidiSelectionModelPtr selection)
{
    it = findNextNoteOrCurrent(track, it);
    if (it == track->end()) {
        selection->clear();
    } else {
        MidiEventPtr evt = it->second;
        if (evt->type == MidiEvent::Type::End) {
            selection->clear();
        } else {
            selection->select(evt);
        }
    }
}
#endif

#if 0
static void selectPrevNoteOrCurrent(
    MidiTrackPtr track,
    MidiTrack::const_iterator it,
    MidiSelectionModelPtr selection)
{
    it = findPrevNoteOrCurrent(track, it);
    if (it == track->end()) {
        // If we can't find a good one, give up
        selection->clear();
    } else {
        MidiEventPtr evt = it->second;
        if (evt->type == MidiEvent::Type::End) {
            selection->clear();
        } else {
            selection->select(evt);
        }
    }
}
#endif

static void selectNextNotePastCursor(bool atCursorOk,
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
        return;
    }

    // if it came back with a note exactly at cursor time,
    // check if it's acceptable.
    if ((it->first > t) || (it->first == t && atCursorOk)) {
        seq->selection->addToSelection(it->second, keepExisting);
        return;
    }

    MidiTrack::const_iterator bestSoFar = it;

    // If must be past cursor time, advance to next
    ++it;

    for (; it != track->end(); ++it) {
        MidiEventPtr evt = it->second;
        if (evt->type == MidiEvent::Type::Note) {
            seq->selection->addToSelection(it->second, keepExisting);
            return;
        }
    }

    // If nothing past where we are, it's ok, even it it is at the same time
    seq->selection->addToSelection(bestSoFar->second, keepExisting);
}

static void selectPrevNoteBeforeCursor(bool atCursorOk,
    MidiSequencerPtr seq)
{
    const auto t = seq->context->cursorTime();
    const auto track = seq->context->getTrack();

    // first, seek into track until cursor time.
    MidiTrack::const_iterator it = track->seekToTimeNote(t);
    if (it == track->end()) {
        seq->selection->clear();
        return;
    }

    // if it came back with a note exactly at cursor time,
    // check if it's acceptable.
    if ((it->first < t) || (it->first == t && atCursorOk)) {
        seq->selection->select(it->second);
        return;
    }

    MidiTrack::const_iterator bestSoFar = it;

    // If must be before cursor time, go back to next
    --it;

    for (; it != track->end(); --it) {
        MidiEventPtr evt = it->second;
        if ((evt->type == MidiEvent::Type::Note) && (evt->startTime < t)) {
            seq->selection->select(evt);
            return;
        }
    }

    // If nothing past where we are, it's ok, even it it is at the same time
    seq->selection->select(bestSoFar->second);
}


#if 0 // abandon this for now
static void selectNextNotePastSelection(MidiSequencerPtr seq)
{
    // get the time of the (only) event in the selection
    assert(seq->selection->size() == 1);
    MidiEventPtr selected = *seq->selection->begin();
    const auto t = selected->startTime;

    // first, seek into track until selection time.
    MidiTrack::const_iterator it = track->seekToTimeNote(t);
    if (it == track->end()) {
        selection->clear();
        return;
    }
    
}
#endif

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
    selectNextNotePastCursor(acceptCursorTime, true, seq());

    updateCursor();
    seq()->context->adjustViewportForCursor();
}
#if 0
void MidiEditor::extendSelectionToNextNote()
{
    printf("extendSelectionToNextNote\n");
    auto sq = seq();
    auto origSelection = sq->selection->clone();
    printf("orig selection had %d\n", sq->selection->size());
    sq->selection->clear();
    selectNextNote();
    for (auto it : *origSelection) {
        printf("adding orig to new\n");
        sq->selection->extendSelection(it);
    }
    printf("leaving extendSelectionToNextNote, have %d\n", sq->selection->size());
    fflush(stdout);
    seq()->assertValid();
}
#endif

void MidiEditor::selectPrevNote()
{
    seq()->assertValid();

    MidiTrackPtr track = getTrack();
    assert(track);
    const bool acceptCursorTime = seq()->selection->empty();
    selectPrevNoteBeforeCursor(acceptCursorTime, seq());
    updateCursor();
    seq()->context->adjustViewportForCursor();
}

void MidiEditor::extendSelectionToPrevNote()
{
    assert(false);
}

// Move to edit context?
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
 //   printf("in updateCursor, moving to note start = %f pitch=%f\n", firstNote->startTime, firstNote->pitchCV); fflush(stdout);
    seq()->context->setCursorTime(firstNote->startTime);
    seq()->context->setCursorPitch(firstNote->pitchCV);
}


void MidiEditor::changePitch(int semitones)
{
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangePitchCommand(seq(), semitones);
    seq()->undo->execute(cmd);
    seq()->assertValid();
    float deltaCV = PitchUtils::semitone * semitones;


    // Now fixup selection and viewport
    seq()->context->setCursorPitch(seq()->context->cursorPitch() + deltaCV);
    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
}

void MidiEditor::changeStartTime(bool ticks, int amount)
{
    MidiLocker l(seq()->song->lock);
    assert(!ticks);         // not implemented yet
    assert(amount != 0);
    float advanceAmount = amount * 1.f / 4.f;       // hard code units to 1/16th notes

#if 1
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangeStartTimeCommand(seq(), advanceAmount);
    seq()->undo->execute(cmd);
    seq()->assertValid();

    // after we change start times, we need to put the cursor on the moved notes
    seq()->context->setCursorToSelection(seq()->selection);
#else

    MidiNoteEventPtr lastNote = safe_cast<MidiNoteEvent>(seq()->selection->getLast());
    float lastTime = lastNote->startTime + lastNote->duration;
    lastTime += advanceAmount;
    extendTrackToMinDuration(lastTime);

    bool setCursor = false;
    MidiTrackPtr track = seq()->context->getTrack();

    for (auto ev : *seq()->selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);       // for now selection is all notes
        track->deleteEvent(*note);
        note->startTime += advanceAmount;
        note->startTime = std::max(0.f, note->startTime);
        track->insertEvent(note);
        if (!setCursor) {
            seq()->context->setCursorTime(note->startTime);
            setCursor = true;
        }
    }
#endif
    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
}

void MidiEditor::changeDuration(bool ticks, int amount)
{
    assert(!ticks);         // not implemented yet
    assert(amount != 0);

    float advanceAmount = amount * 1.f / 4.f;       // hard code units to 1/16th notes

#if 1
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangeDurationCommand(seq(), advanceAmount);
    seq()->undo->execute(cmd);
    seq()->assertValid();
#else

    for (auto ev : *seq()->selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);       // for now selection is all notes
        note->duration += advanceAmount;

        // arbitrary min limit.
        note->duration = std::max(.001f, note->duration);
    }
#endif
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

void MidiEditor::changeCursorPitch(int semitones)
{
    float pitch = seq()->context->cursorPitch() + (semitones * PitchUtils::semitone);
    pitch = std::max(pitch, -5.f);
    pitch = std::min(pitch, 5.f);
    seq()->context->setCursorPitch(pitch);
    seq()->context->scrollViewportToCursorPitch();
    updateSelectionForCursor();
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

void MidiEditor::insertNote()
{
    MidiLocker l(seq()->song->lock);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = seq()->context->cursorTime();
    note->pitchCV = seq()->context->cursorPitch();
    note->duration = 1;  // for now, fixed to quarter
    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq(), note);

    seq()->undo->execute(cmd);

    updateSelectionForCursor();
    seq()->assertValid();
}

void MidiEditor::deleteNote()
{

    if (seq()->selection->empty()) {
        return;
    }

    auto cmd = ReplaceDataCommand::makeDeleteCommand(seq());

    seq()->undo->execute(cmd);
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
    seq()->undo->execute(cmd);
    seq()->assertValid();

    // TODO: what do we select afterwards?
}