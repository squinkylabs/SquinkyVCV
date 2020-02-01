
#include "AuditionLocker.h"
#include "InteropClipboard.h"
#include "ISeqSettings.h"
#include "MidiEditor.h"
#include "MidiEditorContext.h"
#include "MidiLock.h"
#include "MidiSelectionModel.h"
#include "MidiSequencer.h"
#include "MidiSong.h"
#include "MidiTrack.h"
#include "ReplaceDataCommand.h"
#include "SqClipboard.h"
#include "SqMath.h"
#include "TimeUtils.h"

#include <assert.h>

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

void MidiEditor::setCursorToNote(MidiNoteEventPtr note)
{
    if (note) {
        seq()->context->setCursorTime(note->startTime);
        seq()->context->setCursorPitch(note->pitchCV);
    }
}

void MidiEditor::updateCursor()
{
#ifdef _LOG
    printf("updateCursor #sel=%d\n", seq()->selection->size());
#endif
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
#ifdef _LOG
                printf("updateCursor accepting current selection #sel=%d\n", seq()->selection->size());
#endif
                return;
            }
        }
    }
    setCursorToNote(firstNote);
#ifdef _LOG
    printf("updateCursor setting cursor to first note %.2f\n", firstNote->startTime);
#endif
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

    seq()->context->setCursorPitch(newCursorPitch);
    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
}

void MidiEditor::changeStartTime(bool ticks, int amount)
{
    MidiLocker l(seq()->song->lock);
    AuditionLocker u(seq()->selection);       // don't audition while shifting
    assert(amount != 0);

    ISeqSettingsPtr settings = seq()->context->settings();

    // "units" are 16th, "ticks" are 64th
    float advanceAmount = amount * (ticks ? (1.f / 16.f) : settings->getQuarterNotesInGrid());

    const bool snap = seq()->context->settings()->snapToGrid();
    float quantizeGrid = 0;
    if (snap && !ticks) {
        quantizeGrid = settings->getQuarterNotesInGrid();
    }

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangeStartTimeCommand(seq(), advanceAmount, quantizeGrid);
    seq()->undo->execute(seq(), cmd);
    seq()->assertValid();

    // after we change start times, we need to put the cursor on the moved notes
    seq()->context->setCursorToSelection(seq()->selection);
    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
}

void MidiEditor::changeStartTime(const std::vector<float>& shifts)
{
    MidiLocker l(seq()->song->lock);
    AuditionLocker u(seq()->selection);       // don't audition while shifting
    assert(!shifts.empty());

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangeStartTimeCommand(seq(), shifts);
    seq()->undo->execute(seq(), cmd);
    seq()->assertValid();

    // after we change start times, we need to put the cursor on the moved notes
    seq()->context->setCursorToSelection(seq()->selection);
    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
}

void MidiEditor::changeDuration(bool ticks, int amount)
{
    MidiLocker l(seq()->song->lock);
    AuditionLocker u(seq()->selection);       // don't audition while shifting
    assert(amount != 0);

    float advanceAmount = amount * (ticks ? (1.f / 16.f) : (1.f / 4.f));

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangeDurationCommand(seq(), advanceAmount, false);
    seq()->undo->execute(seq(), cmd);
    seq()->assertValid();
}

void MidiEditor::changeDuration(const std::vector<float>& shifts)
{
    MidiLocker l(seq()->song->lock);
    assert(!shifts.empty());
    AuditionLocker u(seq()->selection);       // don't audition while shifting

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangeDurationCommand(seq(), shifts);
    seq()->undo->execute(seq(), cmd);
    seq()->assertValid();
}

void MidiEditor::setDuration(float duration)
{
    MidiLocker l(seq()->song->lock);
    AuditionLocker u(seq()->selection);       // don't audition while shifting
    assert(duration > 0);

    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangeDurationCommand(seq(), duration, true);
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

void MidiEditor::advanceCursorToTime(float time, bool extendSelection)
{
    seq()->context->setCursorTime(std::max(0.f, time));
    updateSelectionForCursor(extendSelection);
    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
    seq()->assertValid();
}

void MidiEditor::advanceCursor(Advance type, int multiplier)
{
    
    assert(multiplier != 0);

    seq()->context->assertCursorInViewport();

    float advanceAmount = 0;
    bool doRelative = true;
    switch (type) {
        case Tick:
            advanceAmount = 4.0 / 64.f;
            doRelative = true;
            break;
        case Beat:
            advanceAmount = 1;
            doRelative = true;
            break;
        case GridUnit:
            advanceAmount = seq()->context->settings()->getQuarterNotesInGrid();
            doRelative = true;
            break;
        case Measure:
            {
                // what bar are we in now?
                float time = seq()->context->cursorTime();
                auto bb = TimeUtils::time2bbf(time);
                int bar = std::get<0>(bb);
                bar += multiplier;          // next one
                bar = std::max(0, bar);
                advanceAmount = TimeUtils::bar2time(bar);
                doRelative = false;
            }
            break;
        case All:
            {
                const float len = getTrack()->getLength();
                auto bb = TimeUtils::time2bbf(len);
                int bar = 0;
                if (multiplier > 0) {
                    // if not even bar, go to the last fractional bar
                    if ((std::get<1>(bb) != 0) || (std::get<2>(bb) != 0)) {
                        bar = std::get<0>(bb);
                    } else {
                        bar = std::get<0>(bb) - 1;
                    }
                }
                advanceAmount = TimeUtils::bar2time(bar);
                doRelative = false;
            }
            break;
        default:
            assert(false);
    }

    if (doRelative) {
        // pre-quantize cursor time, if needed.
        // Don't quantize ticks
        auto newCursorTime = seq()->context->cursorTime();
        if (type != Tick) {
            newCursorTime = seq()->context->settings()->quantize(seq()->context->cursorTime(), true);
        }

        seq()->context->setCursorTime(newCursorTime);

        advanceAmount *= multiplier;
        float newTime = seq()->context->cursorTime() + advanceAmount;
        advanceCursorToTime(newTime, false);
    } else {
        advanceCursorToTime(advanceAmount, false);
    }

    // If the loop points have moved, then
    // lock the MIDI so that a) we can change the loop atomically,
    // and b) so that player realized the model is dirty.

    const SubrangeLoop& origLoop = seq()->song->getSubrangeLoop();
    const bool loopChanged = origLoop.enabled &&
        (origLoop.startTime != seq()->context->startTime() || 
            origLoop.endTime != seq()->context->endTime());

    if (loopChanged) {
        MidiLocker _lock(seq()->song->lock);
        const SubrangeLoop& l = seq()->song->getSubrangeLoop();
        if (l.enabled) {
            SubrangeLoop newLoop(
                l.enabled,
                seq()->context->startTime(),
                seq()->context->endTime());
            seq()->song->setSubrangeLoop(newLoop);
        }
    }
}

void MidiEditor::changeCursorPitch(int semitones)
{
    float pitch = seq()->context->cursorPitch() + (semitones * PitchUtils::semitone);
    setNewCursorPitch(pitch, false);
}

void MidiEditor::setNewCursorPitch(float pitch, bool extendSelection)
{
    pitch = std::max(pitch, -5.f);
    pitch = std::min(pitch, 5.f);
    seq()->context->setCursorPitch(pitch);
    seq()->context->scrollViewportToCursorPitch();
    updateSelectionForCursor(extendSelection);
}


 MidiNoteEventPtr MidiEditor::moveToTimeAndPitch(float time, float pitchCV)
 {
     // make a helper for this combo?
    seq()->context->setCursorPitch(pitchCV);
    seq()->context->scrollViewportToCursorPitch();

    seq()->context->setCursorTime(std::max(0.f, time));
    seq()->context->adjustViewportForCursor();
    seq()->assertValid();

    // if there is no note at the new location, leave
    MidiNoteEventPtr note = getNoteUnderCursor();
    return note;
 }
 
void MidiEditor::selectAt(float time, float pitchCV, bool shiftKey)
{
    // Implement by calling existing handlers. This will
    // cause double update, but I don't think anyone cares.

    // TODO: break up this function
    if (!shiftKey) {
        setNewCursorPitch(pitchCV, false);
        advanceCursorToTime(time, false);
    } else {
        // for shift key, just move the cursor to the new pitch
        seq()->context->setCursorTime(time);
        seq()->context->setCursorPitch(pitchCV);
        // and extend the old selection to include it
        extendSelectionToCurrentNote();
    }
}

void MidiEditor::toggleSelectionAt(float time, float pitchCV)
{
    // set the pitch and time, without messing up selection
    pitchCV = std::max(pitchCV, -5.f);
    pitchCV = std::min(pitchCV, 5.f);
    seq()->context->setCursorPitch(pitchCV);
    seq()->context->scrollViewportToCursorPitch();

    seq()->context->setCursorTime(std::max(0.f, time));
    seq()->context->adjustViewportForCursor();
    seq()->assertValid();

    // if there is no note at the new location, leave
    MidiNoteEventPtr note = getNoteUnderCursor();
    if (!note) {
        return;
    }

    const bool noteIsSelected = seq()->selection->isSelected(note);
    // if the note is not selected, select it
    if (!noteIsSelected) {
        seq()->selection->addToSelection(note, true);
    } else {

        // if it is, remove it from selection
        seq()->selection->removeFromSelection(note);
    }

}

float MidiEditor::getDuration(MidiEditor::Durations dur)
{
    float ret = 0;
    switch (dur) {
        case MidiEditor::Durations::Whole:
            ret = 4;
            break;
        case MidiEditor::Durations::Half:
            ret = 2;
            break;
        case MidiEditor::Durations::Quarter:
            ret = 1;
            break;
        case MidiEditor::Durations::Eighth:
            ret = .5;
            break;
        case MidiEditor::Durations::Sixteenth:
            ret = .25f;
            break;
        default:
            assert(false);
    }
    return ret;
}

float MidiEditor::insertNoteHelper(Durations dur, bool moveCursorAfter)
{
    float duration = getDuration(dur);
    insertNoteHelper2(duration, moveCursorAfter);
    return duration;
}

void MidiEditor::insertNoteHelper2(float dur, bool moveCursorAfter)
{
    const float artic = seq()->context->settings()->articulation();
    assertGT(artic, .001);
    assertLT(artic, 1.1);

    const float cursorAdvance = moveCursorAfter ? dur : 0;
    const float duration = dur * artic;
    insertNoteHelper3(duration, cursorAdvance, false);
}

void MidiEditor::insertNoteHelper3(float duration, float advanceAmount, bool extendSelection)
{
    MidiLocker l(seq()->song->lock);
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();

    note->startTime = seq()->context->cursorTime();
    note->pitchCV = seq()->context->cursorPitch();
    note->duration = duration;
    // DEBUG("inserting note at pitch %.2f. c# = %.2f, b= %.2f", note->pitchCV, PitchUtils::semitone, - PitchUtils::semitone);

    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq(), note, extendSelection);
    seq()->undo->execute(seq(), cmd);
    seq()->context->setCursorTime(note->startTime + advanceAmount);
    updateSelectionForCursor(extendSelection);

     // after we change start times, we need to put the cursor on the moved notes
    seq()->context->setCursorToSelection(seq()->selection);
    seq()->context->adjustViewportForCursor();
    seq()->assertValid();
}

float MidiEditor::insertPresetNote(Durations dur, bool advanceAfter)
{
    return insertNoteHelper(dur, advanceAfter);
}

void MidiEditor::grabDefaultNote()
{
    MidiNoteEventPtr note = getNoteUnderCursor();
    if (note) {
        seq()->context->insertNoteDuration = note->duration;
    }
}

std::pair<float, float> MidiEditor::getDefaultNoteDurationAndAdvance()
{
    float duration = seq()->context->insertNoteDuration;
    float advanceAmount = 0;
    if (duration <= 0) {

        // use grid for default length
        const float artic = seq()->context->settings()->articulation();
        assertGT(artic, .001);
        assertLT(artic, 1.1);

        float totalDuration = seq()->context->settings()->getQuarterNotesInGrid();
        duration = totalDuration * artic;
        advanceAmount = totalDuration;
    } else {
        // this case we have an insert duration
        // need to guess total

        float grid = seq()->context->settings()->getQuarterNotesInGrid();
        advanceAmount = (float) TimeUtils::quantize(duration, grid, false);
        while (advanceAmount < duration) {
            advanceAmount += grid;
        }
    }
    return std::make_pair(duration, advanceAmount);
}


float MidiEditor::getAdvanceTimeAfterNote()
{
    auto x = getDefaultNoteDurationAndAdvance();
    return x.second;
}

// TODO: use the above to calc advance
// ACtually, combine them

float MidiEditor::insertDefaultNote(bool advanceAfter, bool extendSelection)
{
    auto x = getDefaultNoteDurationAndAdvance();
    const float duration = x.first;
    const float advanceAmount = advanceAfter ? x.second : 0;
    insertNoteHelper3(duration, advanceAmount, extendSelection);
    return duration;
}

void MidiEditor::deleteNote()
{
    const char* const name = (seq()->selection->size() > 1) ?
        "delete notes" : "delete note"; 
    deleteNoteSub(name);
}

void MidiEditor::deleteNoteSub(const char* name)
{
    if (seq()->selection->empty()) {
        return;
    }

    auto cmd = ReplaceDataCommand::makeDeleteCommand(seq(), name);

    seq()->undo->execute(seq(), cmd);
    // TODO: move selection into undo
    seq()->selection->clear();
}

/*
new version of updateSelectionForCursor
1) find note under cursor
2) add/replace selection
*/
void MidiEditor::updateSelectionForCursor(bool extendCurrent)
{
    MidiNoteEventPtr note = getNoteUnderCursor();
    if (!note) {
        if (!extendCurrent) {
            seq()->selection->clear();
        }
    } else {
        seq()->selection->addToSelection(note, extendCurrent);
    #ifdef _NEWTAB
        seq()->context->setCursorNote(note);            // for new selection
    #endif
    }
}

MidiNoteEventPtr MidiEditor::getNoteUnderCursor()
{
    const int cursorSemi = PitchUtils::cvToSemitone(seq()->context->cursorPitch());

    // iterate over all the notes that are in the edit context
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
            //seq()->selection->select(note);
            return note;
        }
    }
    return nullptr;
}

void MidiEditor::extendSelectionToCurrentNote()
{
    MidiNoteEventPtr ni = getNoteUnderCursor();

    sq::Rect boundingBox;
    if (ni) {
        // If there is a note at cursor, start our bounding box with that
        sq::Vec initPos = {ni->startTime, ni->pitchCV};
        boundingBox = {initPos, {ni->duration, 0}};
    } else {
        // If no note under cursor, get bounding box of cursor position,
        // but give it non-zero length, so our filter logic works.
        sq::Vec initPos = {seq()->context->cursorTime(), seq()->context->cursorPitch()};
        boundingBox = {initPos, {.001f,0}};
    }

    // expand to include all the notes in selection
    for (auto sel : *seq()->selection) {
        MidiNoteEventPtr n = safe_cast<MidiNoteEvent>(sel);
        if (n) {
            sq::Rect nr{{n->startTime, n->pitchCV}, {n->duration,0}};
            boundingBox = boundingBox.expand(nr);
        }
    }

    // now boundingBox is the final selection area
    // Find all notes in new bounds
    // iterator_pair getEvents(float timeLow, float timeHigh, float pitchLow, float pitchHigh);
    MidiEditorContext::iterator_pair it = seq()->context->getEvents(
        boundingBox.pos.x,
        boundingBox.getRight(),
        boundingBox.pos.y,
        boundingBox.getBottom());

    // iterate all the notes, and add to selection
    for (; it.first != it.second; ++it.first) {
        auto temp = *(it.first);
        MidiEventPtr ev = temp.second;
        MidiNoteEventPtr n = safe_cast<MidiNoteEvent>(ev);
        if (n) {
            seq()->selection->extendSelection(n);
        }
    }
}

void MidiEditor::setNoteEditorAttribute(MidiEditorContext::NoteAttribute attr)
{
    seq()->context->noteAttribute = attr;
}

void MidiEditor::selectAll()
{
    seq()->selection->selectAll(seq()->context->getTrack());
}

void MidiEditor::changeTrackLength()
{
    float endTime = seq()->context->cursorTime();

    if (seq()->context->settings()->snapToGrid()) {
        // if snap to grid is on, snap it
        endTime = seq()->context->settings()->quantizeAlways(endTime, false);
    } else {
        // otherwise, snap to 1/16 note, but round up to the next one.
        const float orig = endTime;
        endTime = (float) TimeUtils::quantize(endTime, .25f, false);
        if (endTime < orig) {
            endTime += .25;
        }
    }
    auto cmd = ReplaceDataCommand::makeMoveEndCommand(seq(), endTime);
    seq()->undo->execute(seq(), cmd);
}

bool MidiEditor::isLooped() const
{
    SubrangeLoop l = seq()->song->getSubrangeLoop(); 
    return l.enabled;
}

void MidiEditor::loop()
{
    MidiLocker _lock(seq()->song->lock);
    SubrangeLoop l = seq()->song->getSubrangeLoop();
    if (l.enabled) {
        l.enabled = false;
    } else {
        l.enabled = true;
        l.startTime = seq()->context->startTime();
        l.endTime = seq()->context->endTime();
    }
    seq()->song->setSubrangeLoop(l);
}

// TODO: use this for copy, too
void moveSelectionToClipboard(MidiSequencerPtr seq)
{
    float earliestEventTime = 0;
    bool firstOne = true;

    MidiTrackPtr track = std::make_shared<MidiTrack>(seq->song->lock);
    for (auto it : *seq->selection) {
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

#ifdef _OLDCLIP
    std::shared_ptr<SqClipboard::Track> clipData = std::make_shared< SqClipboard::Track>();
    clipData->track = track;

    clipData->offset = float(earliestEventTime);
    SqClipboard::putTrackData(clipData);
#else
    InteropClipboard::put(track, seq->selection->isAllSelected());
#endif
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
   
    auto sourceTrack = seq()->context->getTrack();
    const float sourceLength = sourceTrack->getLength();
    track->insertEnd(sourceLength);
    track->assertValid();

#ifdef _OLDCLIP
    
    
    std::shared_ptr<SqClipboard::Track> clipData = std::make_shared<SqClipboard::Track>();
    clipData->track = track;
    clipData->offset = float(earliestEventTime);
    SqClipboard::putTrackData(clipData);
#else
    InteropClipboard::put(track, seq()->selection->isAllSelected());
#endif

#if 0   // old
    // don't copy empty track.
    // for 4x4 we probably want to?
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
    clipData->offset = float(earliestEventTime);
    SqClipboard::putTrackData(clipData);

#endif
   
}

void MidiEditor::paste()
{
 #ifdef _OLDCLIP
    if (!SqClipboard::getTrackData()) {
        return;
    }
#else
    // TODO: this will parse twice!
    if (!InteropClipboard::get()) {
        return;
    }
#endif
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makePasteCommand(seq());
    seq()->undo->execute(seq(), cmd);

    // Am finding that after this cursor pitch is not in view-port
    updateCursor();  
    seq()->context->adjustViewportForCursor();
    seq()->assertValid();

    // TODO: what do we select afterwards?
}

void MidiEditor::cut()
{
    auto songLock = seq()->song->lock;
    MidiLocker l(songLock);

    moveSelectionToClipboard(seq());
    // put all the notes in the clip
    // cut out all the notes in an undoable way
    deleteNoteSub("cut");
}